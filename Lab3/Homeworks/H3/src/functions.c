#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "../include/functions.h"



static const Task kDefaultTasks[N_TASKS] = {
    {0, 7, 0}, {3, 1, 0}, {6, 2, 0}, {4, 0, 0},
    {2, 5, 0}, {7, 3, 0}, {1, 6, 0}, {5, 2, 0},
    {0, 4, 0}, {6, 7, 0}, {2, 0, 0}, {3, 5, 0}
};

static Task g_tasks[N_TASKS];
static Worker g_workers[N_WORKERS];
static Elevator g_elevators[N_ELEVATORS];

static pthread_mutex_t g_state_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond_worker_available = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_cond_elevator_available = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_cond_all_tasks_done = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_cond_worker_task[N_WORKERS];

static int g_tasks_assigned = 0;
static int g_tasks_completed = 0;

/* elevatormoving name is fixed by assignment spec; keep as-is for API. */
void elevatormoving(void)  /* NOLINT */ {
    int ms = 200 + rand() % 401;
    usleep(ms * 1000);
}

void SystemInit(void) {
    pthread_mutex_lock(&g_state_mtx);

    for (int i = 0; i < N_TASKS; i++) {
        g_tasks[i] = kDefaultTasks[i];
    }

    for (int i = 0; i < N_WORKERS; i++) {
        g_workers[i].busy = 0;
        g_workers[i].task_id = -1;
        pthread_cond_init(&g_cond_worker_task[i], NULL);
    }

    for (int i = 0; i < N_ELEVATORS; i++) {
        g_elevators[i].busy = 0;
        g_elevators[i].id = i;
        pthread_mutex_init(&g_elevators[i].mtx, NULL);
    }

    g_tasks_assigned = 0;
    g_tasks_completed = 0;

    pthread_mutex_unlock(&g_state_mtx);
}

void SystemDestroy(void) {
    pthread_mutex_lock(&g_state_mtx);

    for (int i = 0; i < N_ELEVATORS; i++) {
        pthread_mutex_destroy(&g_elevators[i].mtx);
    }

    for (int i = 0; i < N_WORKERS; i++) {
        pthread_cond_destroy(&g_cond_worker_task[i]);
    }

    pthread_mutex_unlock(&g_state_mtx);

    pthread_mutex_destroy(&g_state_mtx);
    pthread_cond_destroy(&g_cond_worker_available);
    pthread_cond_destroy(&g_cond_elevator_available);
    pthread_cond_destroy(&g_cond_all_tasks_done);
}

void* ManagerThread(void* arg) {
    (void)arg;

    pthread_mutex_lock(&g_state_mtx);

    while (g_tasks_assigned < N_TASKS) {
        int free_worker = -1;

        for (int i = 0; i < N_WORKERS; i++) {
            if (g_workers[i].busy == 0) {
                free_worker = i;
                break;
            }
        }

        if (free_worker == -1) {
            pthread_cond_wait(&g_cond_worker_available, &g_state_mtx);
            continue;
        }

        int task_id = -1;
        for (int i = 0; i < N_TASKS; i++) {
            if (g_tasks[i].assigned == 0) {
                task_id = i;
                break;
            }
        }

        if (task_id == -1) {
            break;
        }

        g_tasks[task_id].assigned = 1;
        g_workers[free_worker].busy = 1;
        g_workers[free_worker].task_id = task_id;
        g_tasks_assigned++;

        pthread_cond_signal(&g_cond_worker_task[free_worker]);
    }

    while (g_tasks_completed < N_TASKS) {
        pthread_cond_wait(&g_cond_all_tasks_done, &g_state_mtx);
    }

    for (int i = 0; i < N_WORKERS; i++) {
        pthread_cond_broadcast(&g_cond_worker_task[i]);
    }

    pthread_mutex_unlock(&g_state_mtx);

    return NULL;
}

void* WorkerThread(void* arg) {
    int my_id = *(int *)arg;

    while (1) {
        pthread_mutex_lock(&g_state_mtx);

        while (g_workers[my_id].task_id == -1 && g_tasks_completed < N_TASKS) {
            pthread_cond_wait(&g_cond_worker_task[my_id], &g_state_mtx);
        }

        if (g_workers[my_id].task_id == -1 && g_tasks_completed >= N_TASKS) {
            pthread_mutex_unlock(&g_state_mtx);
            break;
        }

        int task_id = g_workers[my_id].task_id;
        g_workers[my_id].task_id = -1;

        int pick = g_tasks[task_id].pick_floor;
        int drop = g_tasks[task_id].drop_floor;

        pthread_mutex_unlock(&g_state_mtx);

        int elev_index = -1;

        while (elev_index == -1) {
            for (int i = 0; i < N_ELEVATORS; i++) {
                if (pthread_mutex_trylock(&g_elevators[i].mtx) == 0) {
                    elev_index = i;
                    break;
                }
            }

            if (elev_index == -1) {
                pthread_mutex_lock(&g_state_mtx);
                pthread_cond_wait(&g_cond_elevator_available, &g_state_mtx);
                pthread_mutex_unlock(&g_state_mtx);
            }
        }

        pthread_mutex_lock(&g_state_mtx);
        g_elevators[elev_index].busy = 1;
        int elev_id = g_elevators[elev_index].id;
        pthread_mutex_unlock(&g_state_mtx);

        printf("WORKER %d EXECUTING TASK %d | PICK %d -> DROP %d | "
               "ELEVATOR %d\n",
               my_id, task_id, pick, drop, elev_id);

        elevatormoving();

        printf("WORKER %d FINISHED TASK %d | PICKED %d | DROPPED %d | "
               "ELEVATOR %d\n",
               my_id, task_id, pick, drop, elev_id);

        pthread_mutex_lock(&g_state_mtx);

        g_elevators[elev_index].busy = 0;
        g_workers[my_id].busy = 0;
        g_tasks_completed++;

        if (g_tasks_completed >= N_TASKS) {
            pthread_cond_broadcast(&g_cond_all_tasks_done);
            for (int i = 0; i < N_WORKERS; i++) {
                pthread_cond_broadcast(&g_cond_worker_task[i]);
            }
        }

        pthread_cond_signal(&g_cond_worker_available);
        pthread_cond_signal(&g_cond_elevator_available);

        pthread_mutex_unlock(&g_state_mtx);

        pthread_mutex_unlock(&g_elevators[elev_index].mtx);
    }

    return NULL;
}