#include "../include/functions.h"

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

int ReadInstructions(Instruction *plan) {
  FILE *file = fopen("instrucciones.txt", "r");
  if (!file) {
    perror("Error al abrir instrucciones.txt");
    return -1;
  }

  int current_burst = 0;
  // Tu misma lógica de fscanf, guardando en el arreglo 'plan'
  while (current_burst < 30 &&
         fscanf(file, "%s %d %d %d", plan[current_burst].policyStr,
                &plan[current_burst].p1, &plan[current_burst].p2,
                &plan[current_burst].p3) == 4) {
    current_burst++;
  }

  fclose(file);
  return current_burst;  // Nos dice cuántas líneas leyó (máximo 30)
}

void HeavyTask(int id, int64_t iterations) {
  int64_t count = 0;
  unsigned int seed = (unsigned int)time(NULL) ^ (getpid() << 16) ^ id;

  for (int64_t i = 0; i < iterations; i++) {
    double x = (double)rand_r(&seed) / RAND_MAX;
    double y = (double)rand_r(&seed) / RAND_MAX;
    if (x * x + y * y <= 1.0) count++;
  }
}

double GetTimeDiff(struct timespec start, struct timespec end) {
  return (double)(end.tv_sec - start.tv_sec) +
         (double)(end.tv_nsec - start.tv_nsec) / 1e9;
}

int GetPolicyMacro(char *policy_name) {
  if (strcmp(policy_name, "FIFO") == 0) return SCHED_FIFO;
  if (strcmp(policy_name, "RR") == 0) return SCHED_RR;
  return SCHED_OTHER;
}