#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include "../include/functions.h"

int main() {
    srand(time(NULL));
    signal(SIGUSR2, HandlerSigUSR2);

    int shm_fd = shm_open(SHM_NAME,
                           O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(shm_fd, sizeof(Sector) * NUM_SECTORS);

    Sector *shared_map = mmap(NULL,
        sizeof(Sector) * NUM_SECTORS,
        PROT_READ | PROT_WRITE,
        MAP_SHARED, shm_fd, 0);
    if (shared_map == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    for (int i = 0; i < NUM_SECTORS; i++) {
        shared_map[i].status = 0;
        shared_map[i].drone_pid = 0;
    }

    pid_t drone_pids[NUM_DRONES];
    for (int i = 0; i < NUM_DRONES; i++) {
        drone_pids[i] = fork();
        if (drone_pids[i] == -1) {
            perror("fork");
            exit(1);
        }
        if (drone_pids[i] == 0) {
            execl("../../drone/build/drone",
                  "drone", NULL);
            perror("exec");
            exit(1);
        }
    }

    sleep(1);

    int all_scanned = 0;
    int sig_count = 0;
    while (!all_scanned) {
        int idx;
        if (sig_count < NUM_DRONES) {
            idx = sig_count;
        } else {
            idx = rand() % NUM_DRONES;
        }
        kill(drone_pids[idx], SIGUSR1);
        sig_count++;
        usleep(100000);

        all_scanned = 1;
        for (int i = 0; i < NUM_SECTORS; i++) {
            if (shared_map[i].status == 0) {
                all_scanned = 0;
                break;
            }
        }
    }

    usleep(200000);
    printf("Scan completed successfully.\n");
    for (int i = 0; i < NUM_SECTORS; i++) {
        printf("Sector %d: Drone [%d]\n",
               i, shared_map[i].drone_pid);
    }

    for (int i = 0; i < NUM_DRONES; i++) {
        kill(drone_pids[i], SIGTERM);
        waitpid(drone_pids[i], NULL, 0);
    }

    munmap(shared_map, sizeof(Sector) * NUM_SECTORS);
    shm_unlink(SHM_NAME);
    return 0;
}
