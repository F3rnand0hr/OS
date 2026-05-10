#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>

#include "../include/functions.h"

enum { kExpectedBursts = 30, kChildrenPerBurst = 3 };
static const int64_t k_default_iterations = 30000000;

static int PinSelfToCore0(void) {
#ifdef __linux__
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(0, &set);
  if (sched_setaffinity(0, sizeof(set), &set) != 0) {
    perror("sched_setaffinity");
    return -1;
  }
  return 0;
#else
  return 0;
#endif
}

static void ApplyNiceValueOrWarn(int nice_value) {
  if (setpriority(PRIO_PROCESS, 0, nice_value) != 0) {
    perror("setpriority");
  }
}

static int ClampInt(int x, int lo, int hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

static void ApplySchedulerOrWarn(int policy_macro, int requested_priority) {
#ifdef __linux__
  struct sched_param param;
  memset(&param, 0, sizeof(param));

  if (policy_macro == SCHED_FIFO || policy_macro == SCHED_RR) {
    int minp = sched_get_priority_min(policy_macro);
    int maxp = sched_get_priority_max(policy_macro);
    if (minp < 1) minp = 1;
    if (maxp < minp) maxp = minp;
    param.sched_priority = ClampInt(requested_priority, minp, maxp);
  } else {
    param.sched_priority = 0;
  }

  if (sched_setscheduler(0, policy_macro, &param) != 0) {
    perror("sched_setscheduler");
  }
#else
  (void)policy_macro;
  (void)requested_priority;
#endif
}

static int StatsIndexForPolicyStr(const char* policy_str) {
  if (strcmp(policy_str, "FIFO") == 0) return 0;
  if (strcmp(policy_str, "RR") == 0) return 1;
  return 2;
}

static void ConfigureChild(int policy_macro, int nice_value) {
  if (PinSelfToCore0() != 0) exit(2);
  ApplyNiceValueOrWarn(nice_value);
  ApplySchedulerOrWarn(policy_macro, nice_value);
}

int main(int argc, char** argv) {
  int64_t iterations = k_default_iterations;
  if (argc >= 2) iterations = atoll(argv[1]);

  Instruction plan[kExpectedBursts];
  int bursts = ReadInstructions(plan);
  if (bursts != kExpectedBursts) {
    fprintf(stderr, "Error: se esperaban %d instrucciones, se leyeron %d.\n",
            kExpectedBursts, bursts);
    return 1;
  }

  if (PinSelfToCore0() != 0) return 1;

  PolicyStats stats[3];
  memset(stats, 0, sizeof(stats));
  strncpy(stats[0].name, "FIFO", sizeof(stats[0].name) - 1);
  strncpy(stats[1].name, "RR", sizeof(stats[1].name) - 1);
  strncpy(stats[2].name, "OTHER", sizeof(stats[2].name) - 1);

  printf("Initiating Benchmark with %d instructions...\n", kExpectedBursts);

  for (int i = 0; i < kExpectedBursts; i++) {
    int stats_index = StatsIndexForPolicyStr(plan[i].policyStr);
    const char* display_policy = stats[stats_index].name;
    int policy_macro = GetPolicyMacro(plan[i].policyStr);
    int nice_values[3] = {plan[i].p1, plan[i].p2, plan[i].p3};
    pid_t pids[3];

    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    printf("[%d/%d] Ejecutando %s... ", i + 1, kExpectedBursts, display_policy);
    fflush(stdout);

    for (int c = 0; c < kChildrenPerBurst; c++) {
      pid_t pid = fork();
      if (pid < 0) {
        perror("fork");
        return 1;
      }
      if (pid == 0) {
        ConfigureChild(policy_macro, nice_values[c]);
        HeavyTask(c + 1, iterations);
        exit(0);
      }
      pids[c] = pid;
    }

    for (int c = 0; c < kChildrenPerBurst; c++) {
      int status = 0;
      if (waitpid(pids[c], &status, 0) < 0) {
        perror("waitpid");
        return 1;
      }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = GetTimeDiff(start, end);
    printf("Completado en %.4fs\n", elapsed);

    stats[stats_index].total_burst_time += elapsed;
    stats[stats_index].count++;
  }

  printf("\nANÁLISIS DE RENDIMIENTO DEL SCHEDULER\n");
  printf("-------------------------------------------\n");
  printf("%-10s | %-10s | %-12s\n", "POLITICA", "TOTAL(s)", "PROMEDIO(s)");
  printf("-------------------------------------------\n");
  for (int i = 0; i < 3; i++) {
    double avg = (stats[i].count > 0)
                     ? (stats[i].total_burst_time / (double)stats[i].count)
                     : 0.0;
    printf("%-10s | %-10.4f | %-12.4f\n", stats[i].name,
           stats[i].total_burst_time, avg);
  }

  const char* best = "N/A";
  double best_avg = 0.0;
  for (int i = 0; i < 3; i++) {
    if (stats[i].count <= 0) continue;
    double avg = stats[i].total_burst_time / (double)stats[i].count;
    if (strcmp(best, "N/A") == 0 || avg < best_avg) {
      best = stats[i].name;
      best_avg = avg;
    }
  }
  printf("\nRECOMENDACIÓN FINAL: El algoritmo %s fue el más eficiente para "
         "este tipo de carga de trabajo (CPU-Bound).\n",
         best);

  return 0;
}
