#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "../include/functions.h"

int main(void) {
    SystemInit();
    srand(0);

    pthread_t manager;
    pthread_t workers[N_WORKERS];
    int worker_ids[N_WORKERS];

    pthread_create(&manager, NULL, ManagerThread, NULL);

    for (int i = 0; i < N_WORKERS; i++) {
        worker_ids[i] = i;
        pthread_create(&workers[i], NULL, WorkerThread, &worker_ids[i]);
    }

    for (int i = 0; i < N_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }

    pthread_join(manager, NULL);

    SystemDestroy();
    return 0;
}