#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Definición manual por si el header del sistema falla (evita el "incomplete
// type")
struct sched_param_local {
  int sched_priority;
};

typedef struct {
  char policyStr[10];  // Algorithym
  int p1, p2, p3;      // Prioririty
} Instruction;

typedef struct {
  char name[10];
  double total_burst_time;
  int count;
} PolicyStats;

// Prototipos en CamelCase
int ReadInstructions(Instruction* plan);
void HeavyTask(int id, int64_t iterations);
double GetTimeDiff(struct timespec start, struct timespec end);
int GetPolicyMacro(char* policy_name);

#endif  //  INCLUDE_FUNCTIONS_H_