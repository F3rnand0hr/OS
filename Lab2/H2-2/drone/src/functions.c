#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/functions.h"

Sector *shared_map;
int can_scan = 0;

void HandleSigUSR1(int sig) {
    (void)sig;
    pid_t my_pid = getpid();
    int found = -1;
    int scanned = 0;

    for (int i = 0; i < NUM_SECTORS; i++) {
        if (shared_map[i].status == 0 && found == -1) {
            shared_map[i].status = 1;
            shared_map[i].drone_pid = my_pid;
            found = i;
            scanned++;
        } else if (shared_map[i].status == 1) {
            scanned++;
        }
    }

    if (found != -1) {
        printf("[DRONE] %d/%d sectors scanned\n",
               scanned, NUM_SECTORS);
        printf("[DRONE] Last sector scanned:"
               " %d by drone [%d]\n\n",
               found, my_pid);
        kill(getppid(), SIGUSR2);
    }
    can_scan = 0;
}
