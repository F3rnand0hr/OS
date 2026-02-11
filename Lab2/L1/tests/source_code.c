#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }

    pid_t pid = atoi(argv[1]);
    printf("Received PID: %d\n", pid);

    struct sysinfo info;
    sleep(2); // Simulate some processing delay
    if (sysinfo(&info) == 0) {
        printf("Uptime (%d): %ld seconds\n",  pid, info.uptime);
        printf("Total RAM (%d): %ld MB\n",  pid, info.totalram / (1024 * 1024));
        printf("Number of CPUs (%d): %d\n", pid, get_nprocs());
    } else {
        perror("sysinfo");
        return 1;
    }

    return 0;
}