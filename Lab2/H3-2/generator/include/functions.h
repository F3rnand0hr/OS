// Copyright 2025
#ifndef GENERATOR_INCLUDE_FUNCTIONS_H_
#define GENERATOR_INCLUDE_FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

#define SHM_PIDS "/shm_pids"
#define SHM_NAME "/shm_data"

struct Registry {
    pid_t generator_pid;
    pid_t worker_pid;
};

struct Batch {
    int image[5][10][10];
    int type[5];
};

#endif  // GENERATOR_INCLUDE_FUNCTIONS_H_
