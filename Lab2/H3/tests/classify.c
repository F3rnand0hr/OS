#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#define SHM_PIDS "/shm_pids"
#define SHM_NAME "/shm_data"

struct Registry {
    pid_t generator_pid;
    pid_t worker_pid;
};

struct Batch {
    int image[5][10][10];
    int type[2];          
};

int main() {
    int fd_data = shm_open(SHM_NAME, O_RDWR, 0666);
    struct Batch *data = mmap(NULL, sizeof(struct Batch), PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);

    printf("[CLASSIFY] Iniciando clasificación lógica...\n");
    for (int i = 0; i < 5; i++) {
        int sum = 0;
        printf("\n[CLASSIFY] IMAGE %d \n", i+1);
        for (int j = 0; j < 10; j++)
            for (int k = 0; k < 10; k++){
                sum += data->image[i][j][k];
                printf("%d ", data->image[i][j][k]);
            }
        data->type[i] = (sum / 100 >= 50) ? 1 : 2;
        printf("\n[CLASSIFY] IMAGE %d -> %s\n", i+1, (data->type[i] == 1 ? "IMAGEN" : "RUIDO"));
    }
    
    munmap(data, sizeof(struct Batch));
    return 0; 
}