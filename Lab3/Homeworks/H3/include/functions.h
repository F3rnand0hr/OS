#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#include <pthread.h>

#define FLOORS 8
#define N_TASKS 12
#define N_WORKERS 4
#define N_ELEVATORS 2

typedef struct {
    int pick_floor;
    int drop_floor;
    int assigned;
} Task;

typedef struct {
    int busy;
    int task_id;
} Worker;

typedef struct {
    pthread_mutex_t mtx;
    int busy;
    int id;
} Elevator;

/* elevatormoving name is fixed by assignment spec; keep as-is for API. */
void elevatormoving(void);  /* NOLINT */

void SystemInit(void);
void SystemDestroy(void);

void* ManagerThread(void* arg);
void* WorkerThread(void* arg);

#endif  // INCLUDE_FUNCTIONS_H_