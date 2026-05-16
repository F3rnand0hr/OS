#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "functions.h"

static atomic_int pending_workers;

static void* ThreadWrapper(void* arg) {
  Mensaje* msg = (Mensaje*)arg;
  atomic_fetch_add_explicit(&pending_workers, 1, memory_order_relaxed);
  int tid = (int)syscall(SYS_gettid);

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET((unsigned)msg->core, &cpuset);
  if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) != 0) {
    perror("pthread_setaffinity_np");
  }

  if (setpriority(PRIO_PROCESS, 0, msg->prioridad) != 0) {
    perror("setpriority");
  }

  /* Allow the kernel to migrate this thread to the requested CPU */
  sched_yield();
  usleep(2000);

  int actual_core = sched_getcpu();
  const char* confirm =
      (actual_core == msg->core) ? "[CONFIRMED]" : "[MISMATCH]";

  printf(
      "[THREAD | TID: %d] Task %d -> Core Requested: %d | Core Actual: %d "
      "%s\n",
      tid, msg->id_tarea, msg->core, actual_core, confirm);

  switch (msg->tipo_operacion) {
    case 1:
      ExecutePrimes(msg->carga);
      break;
    case 2:
      ExecuteTaylorSeries(msg->carga);
      break;
    case 3:
      ExecuteChecksum(msg->carga);
      break;
    default:
      break;
  }

  atomic_fetch_sub_explicit(&pending_workers, 1, memory_order_relaxed);
  free(msg);
  return NULL;
}

int main(void) {
  struct mq_attr qattr;
  memset(&qattr, 0, sizeof(qattr));
  qattr.mq_maxmsg = 32;
  qattr.mq_msgsize = sizeof(Mensaje);

  mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0666, &qattr);
  if (mq == (mqd_t)-1) {
    perror("mq_open");
    return 1;
  }

  printf("[SCHEDULER] Multicore System active. Waiting for threads...\n");

  for (;;) {
    Mensaje incoming;
    ssize_t received =
        mq_receive(mq, (char*)&incoming, sizeof(incoming), NULL);
    if (received < 0) {
      perror("mq_receive");
      continue;
    }

    if (incoming.tipo_operacion == -1) {
      printf("[SCHEDULER] Shutdown received.\n");
      break;
    }

    Mensaje* task = malloc(sizeof(Mensaje));
    if (task == NULL) {
      perror("malloc");
      continue;
    }
    *task = incoming;

    pthread_t thr;
    pthread_attr_t thr_attr;
    pthread_attr_init(&thr_attr);
    pthread_attr_setdetachstate(&thr_attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(&thr, &thr_attr, ThreadWrapper, task) != 0) {
      perror("pthread_create");
      free(task);
      pthread_attr_destroy(&thr_attr);
      continue;
    }
    pthread_attr_destroy(&thr_attr);
  }

  while (atomic_load_explicit(&pending_workers, memory_order_relaxed) > 0) {
    usleep(500);
  }

  mq_close(mq);
  if (mq_unlink(QUEUE_NAME) == -1 && errno != ENOENT) {
    perror("mq_unlink");
  }

  return 0;
}
