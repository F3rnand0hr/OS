#include <stdio.h>
#include "../include/functions.h"

volatile sig_atomic_t worker_done = 0;

void HandleSigUSR2(int sig) {
  (void)sig;
  worker_done = 1;
}

int main() {
  srand(time(NULL));
  setbuf(stdout, NULL);

  struct sigaction sa;
  sa.sa_handler = HandleSigUSR2;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGUSR2, &sa, NULL);

  int fd_pids = shm_open(SHM_PIDS, O_CREAT | O_RDWR, 0666);
  ftruncate(fd_pids, sizeof(struct Registry));
  struct Registry *registry = mmap(NULL, sizeof(struct Registry),
      PROT_READ | PROT_WRITE, MAP_SHARED, fd_pids, 0);

  int fd_data = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  ftruncate(fd_data, sizeof(struct Batch));
  struct Batch *data = mmap(NULL, sizeof(struct Batch),
      PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);

  registry->generator_pid = getpid();
  registry->worker_pid = 0;

  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 10; j++)
      for (int k = 0; k < 10; k++)
        data->image[i][j][k] = rand() % 100;

  pid_t pid = fork();
  if (pid == 0) {
    execlp("../../tests/classify", "classify", NULL);
    perror("execlp");
    exit(1);
  }
  waitpid(pid, NULL, 0);

  printf("[GENERATOR] Classification ready. Waiting Worker...\n");

  while (registry->worker_pid == 0) {
    usleep(100000);
  }

  pid_t worker_pid = registry->worker_pid;
  printf("[Generator] Sending SIGUSR1 to Worker (PID: %d)...\n", worker_pid);
  kill(worker_pid, SIGUSR1);

  while (!worker_done) {
    pause();
  }
  printf("[Generator] Worker finished.\n");

  munmap(registry, sizeof(struct Registry));
  munmap(data, sizeof(struct Batch));
  close(fd_pids);
  close(fd_data);
  shm_unlink(SHM_PIDS);
  shm_unlink(SHM_NAME);

  return 0;
}
