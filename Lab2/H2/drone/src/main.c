#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/functions.h"

int main() {
    struct sigaction sa;
    sa.sa_handler = HandleSigUSR1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Drone: shm_open");
        exit(1);
    }
    shared_map = mmap(NULL,
        sizeof(Sector) * NUM_SECTORS,
        PROT_READ | PROT_WRITE,
        MAP_SHARED, shm_fd, 0);

    while (1) {
        pause();
    }

    return 0;
}
