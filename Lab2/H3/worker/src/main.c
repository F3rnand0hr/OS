  // Copyright 2025
  #include <stdio.h>
  #include "../include/functions.h"

  volatile sig_atomic_t signal_received = 0;

  void HandleSigUSR1(int sig) {
    (void)sig;
    signal_received = 1;
  }

  int main() {
    setbuf(stdout, NULL);

    struct sigaction sa;
    sa.sa_handler = HandleSigUSR1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    int fd_pids = shm_open(SHM_PIDS, O_RDWR, 0666);
    struct Registry *registry = mmap(NULL, sizeof(struct Registry),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd_pids, 0);

    int fd_data = shm_open(SHM_NAME, O_RDWR, 0666);
    struct Batch *data = mmap(NULL, sizeof(struct Batch),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);

    registry->worker_pid = getpid();
    printf("[Worker] Registred. Waiting SIGUSR1 from Generator...\n");

    while (!signal_received) {
      pause();
    }
    printf("[Worker] starting processing...\n");

    for (int i = 0; i < 5; i++) {
      printf("\n[WORKER] IMAGE %d\n", i + 1);
      if (data->type[i] == 1) {
        for (int j = 0; j < 10; j++)
          for (int k = 0; k < 10; k++)
            data->image[i][j][k] += 10;
      } else {
        for (int j = 0; j < 10; j++)
          for (int k = 0; k < 10; k++)
            data->image[i][j][k] = 0;
      }
      for (int j = 0; j < 10; j++)
        for (int k = 0; k < 10; k++)
          printf("%d ", data->image[i][j][k]);
      printf("\n");
    }

    printf("[Worker] Procesamiento completo. Notificando al Generator...\n");
    kill(registry->generator_pid, SIGUSR2);

    munmap(registry, sizeof(struct Registry));
    munmap(data, sizeof(struct Batch));
    close(fd_pids);
    close(fd_data);

    return 0;
  }
