#ifndef COMMAND_CENTER_INCLUDE_FUNCTIONS_H_
#define COMMAND_CENTER_INCLUDE_FUNCTIONS_H_

#include <sys/types.h>

#define SHM_NAME "/space_terrain_shm"
#define NUM_SECTORS 10
#define NUM_DRONES 3

typedef struct {
    int status;
    pid_t drone_pid;
} Sector;

void HandlerSigUSR2(int sig);

#endif  // COMMAND_CENTER_INCLUDE_FUNCTIONS_H_
