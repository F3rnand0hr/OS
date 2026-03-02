#ifndef DRONE_INCLUDE_FUNCTIONS_H_
#define DRONE_INCLUDE_FUNCTIONS_H_

#include <sys/types.h>

#define SHM_NAME "/space_terrain_shm"
#define NUM_SECTORS 10

typedef struct {
    int status;
    pid_t drone_pid;
} Sector;

extern Sector *shared_map;
extern int can_scan;

void HandleSigUSR1(int sig);

#endif  // DRONE_INCLUDE_FUNCTIONS_H_
