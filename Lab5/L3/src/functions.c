#define _POSIX_C_SOURCE 199309L

#include "../include/functions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t g_pending_tasks = 0;

static void TimerHandler(int signo, siginfo_t* info, void* context) {
  (void)signo;
  (void)context;

  int task_id = info->si_value.sival_int;
  if (task_id < 0 || task_id >= MAX_TAREAS_DICCIONARIO) {
    return;
  }

  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  long frac_ms = ts.tv_nsec / 1000000L;

  printf("[%ld.%03lds] EXECUTING: %s (ID Dict: %d)\n", (long)ts.tv_sec,
         frac_ms, TASK_DICTIONARY[task_id], task_id);

  g_pending_tasks--;
}

void SetupSignalHandler(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = TimerHandler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR1, &sa, NULL);
}

void SetPendingTaskCount(int count) { g_pending_tasks = (sig_atomic_t)count; }

int ScheduleTask(int task_id, double delay_seconds) {
  timer_t timer_id;
  struct sigevent sev;
  memset(&sev, 0, sizeof(sev));
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGUSR1;
  sev.sigev_value.sival_int = task_id;

  if (timer_create(CLOCK_MONOTONIC, &sev, &timer_id) != 0) {
    perror("timer_create");
    return -1;
  }

  struct itimerspec its;
  memset(&its, 0, sizeof(its));
  its.it_value.tv_sec = (time_t)delay_seconds;
  its.it_value.tv_nsec =
      (long)((delay_seconds - (double)its.it_value.tv_sec) * 1e9);
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;

  if (timer_settime(timer_id, 0, &its, NULL) != 0) {
    perror("timer_settime");
    return -1;
  }

  return 0;
}

void WaitForTasks(void) {
  while (g_pending_tasks > 0) {
    pause();
  }
}
