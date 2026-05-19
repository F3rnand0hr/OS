#include <stdio.h>
#include <stdlib.h>

#include "../include/functions.h"

void SetupSignalHandler(void);
void SetPendingTaskCount(int count);
int ScheduleTask(int task_id, double delay_seconds);
void WaitForTasks(void);

int main(int argc, char* argv[]) {
  if (argc < 3 || (argc - 1) % 2 != 0) {
    fprintf(stderr, "Usage: %s <Task_ID> <Time> ...\n", argv[0]);
    return 1;
  }

  int num_tasks = (argc - 1) / 2;

  SetupSignalHandler();
  SetPendingTaskCount(num_tasks);

  for (int i = 1; i < argc; i += 2) {
    int task_id = atoi(argv[i]);
    double delay_seconds = atof(argv[i + 1]);

    if (task_id < 0 || task_id >= MAX_TAREAS_DICCIONARIO) {
      fprintf(stderr, "Invalid task ID: %d\n", task_id);
      return 1;
    }

    if (ScheduleTask(task_id, delay_seconds) != 0) {
      return 1;
    }
  }

  WaitForTasks();
  printf("[System] Scheduling completed.\n");

  return 0;
}
