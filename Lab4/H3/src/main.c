#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../include/functions.h"

#define OUTPUT_REL_PATH "output/resultado_blur.pgm"

static const char *PolicyLabel(int policy_arg) {
  switch (policy_arg) {
    case 0:
      return "OTHER";
    case 1:
      return "FIFO";
    case 2:
      return "RR";
    default:
      return "?";
  }
}

static int MapSchedPolicy(int policy_arg, int *out_policy) {
  switch (policy_arg) {
    case 0:
      *out_policy = SCHED_OTHER;
      return 0;
    case 1:
      *out_policy = SCHED_FIFO;
      return 0;
    case 2:
      *out_policy = SCHED_RR;
      return 0;
    default:
      return -1;
  }
}

static void EnsureOutputDir(void) {
  if (mkdir("output", 0755) != 0 && errno != EEXIST) {
    perror("mkdir output");
    exit(1);
  }
}

static void DistributeRows(thread_data_t *td, unsigned char *in,
                           unsigned char *out, int w, int h, int threads) {
  int base = h / threads;
  int extra = h % threads;
  int row = 0;

  for (int i = 0; i < threads; ++i) {
    int slice = base + (i < extra ? 1 : 0);
    td[i].input = in;
    td[i].output = out;
    td[i].width = w;
    td[i].height = h;
    td[i].start_row = row;
    td[i].end_row = row + slice;
    row += slice;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Uso: %s <entrada.pgm> <hilos> <politica 0..2>\n",
            argv[0]);
    return 1;
  }

  int num_threads = atoi(argv[2]);
  int policy_arg = atoi(argv[3]);
  if (num_threads < 1) {
    fprintf(stderr, "El numero de hilos debe ser >= 1.\n");
    return 1;
  }

  int sched_policy = 0;
  if (MapSchedPolicy(policy_arg, &sched_policy) != 0) {
    fprintf(stderr, "Politica invalida (use 0, 1 o 2).\n");
    return 1;
  }

  int width = 0;
  int height = 0;
  unsigned char *input = LoadPgm(argv[1], &width, &height);
  unsigned char *output =
      (unsigned char *)malloc((size_t)width * (size_t)height);
  if (!output) {
    free(input);
    fprintf(stderr, "Sin memoria para buffer de salida.\n");
    return 1;
  }

  thread_data_t *td =
      calloc((size_t)num_threads, sizeof(thread_data_t));
  pthread_t *tids = calloc((size_t)num_threads, sizeof(pthread_t));
  if (!td || !tids) {
    free(input);
    free(output);
    free(td);
    free(tids);
    fprintf(stderr, "Sin memoria para hilos.\n");
    return 1;
  }

  DistributeRows(td, input, output, width, height, num_threads);

  pthread_attr_t attr;
  const int use_explicit_sched = (policy_arg != 0);
  pthread_attr_t *attrp = NULL;

  if (use_explicit_sched) {
    if (pthread_attr_init(&attr) != 0) {
      perror("pthread_attr_init");
      goto cleanup;
    }
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0) {
      perror("pthread_attr_setinheritsched");
      pthread_attr_destroy(&attr);
      goto cleanup;
    }
    if (pthread_attr_setschedpolicy(&attr, sched_policy) != 0) {
      perror("pthread_attr_setschedpolicy");
      pthread_attr_destroy(&attr);
      goto cleanup;
    }
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    param.sched_priority = sched_get_priority_max(sched_policy);
    if (pthread_attr_setschedparam(&attr, &param) != 0) {
      perror("pthread_attr_setschedparam");
      pthread_attr_destroy(&attr);
      goto cleanup;
    }
    attrp = &attr;
  }

  struct timespec t0 = {0, 0};
  struct timespec t1 = {0, 0};
  clock_gettime(CLOCK_MONOTONIC, &t0);

  for (int i = 0; i < num_threads; ++i) {
    if (pthread_create(&tids[i], attrp, ThreadFunc, &td[i]) != 0) {
      perror("pthread_create");
      /* Join already-created threads before exit. */
      for (int j = 0; j < i; ++j) {
        (void)pthread_join(tids[j], NULL);
      }
      if (use_explicit_sched) {
        pthread_attr_destroy(&attr);
      }
      goto cleanup;
    }
  }

  for (int i = 0; i < num_threads; ++i) {
    (void)pthread_join(tids[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  if (use_explicit_sched) {
    pthread_attr_destroy(&attr);
  }

  double elapsed = (double)(t1.tv_sec - t0.tv_sec) +
                   (double)(t1.tv_nsec - t0.tv_nsec) / 1.0e9;

  EnsureOutputDir();
  SavePgm(OUTPUT_REL_PATH, output, width, height);

  printf("--- Reporte de Ejecucion ---\n");
  printf("Imagen: %s (%dx%d)\n", argv[1], width, height);
  printf("Hilos: %d | Politica: %s\n", num_threads, PolicyLabel(policy_arg));
  printf("Tiempo total: %.6f segundos\n", elapsed);
  printf("-----------------------------\n");

  free(input);
  free(output);
  free(td);
  free(tids);
  return 0;

cleanup:
  free(input);
  free(output);
  free(td);
  free(tids);
  return 1;
}
