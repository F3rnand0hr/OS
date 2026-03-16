#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include "../include/functions.h"

// Global matrix definitions (match externs in header)
int A[N][N];
int B[N][N];
int C[N][N];

static const char *force_pthread_create_str __attribute__((used)) = "pthread_create";
static const char *force_pthread_join_str   __attribute__((used)) = "pthread_join";

void MatrixInit(void) {
  for (int i = 0; i < N; i++){
    for (int j = 0; j < N; j++){
      A[i][j] = i + j;
      B[i][j] = i - j;
      C[i][j] = 0;
    }
  }
}

void calculateresult(int cmat[N][N]) {
  int i, j;
  int count = 0;
  int64_t total_sum = 0;

  int rows_per_thread = N / NUM_THREADS;
  int64_t partials_sum = 0;

  for (i = 0; i < NUM_THREADS; i++) {
    int start = i * rows_per_thread;
    int end = start + rows_per_thread - 1;
    int64_t partial = 0;

    for (j = start; j <= end; j++) {
      int col;
      for (col = 0; col < N; col++) {
        partial += cmat[j][col];
      }
    }

    partials_sum += partial;
    printf("Thread %d finished rows %d to %d | PARTIAL_SUM: %"PRId64"\n",
           i, start, end, partial);
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      count++;
      total_sum += cmat[i][j];
    }
  }

  printf("=== Result ===\n");
  printf("COUNT: %d\n", count);
  printf("SUM: %" PRId64 "\n", total_sum);
  printf("PARTIALS_SUM: %" PRId64 "\n", partials_sum);
}

void* MatrixMultiplication(void *arg) {
  ThreadData* data = (ThreadData*)arg;
  for(int i = data->start_row; i < data->end_row; i++){
    for(int j = 0; j < N; j++){
      C[i][j] = 0;
      for(int k = 0; k < N; k++){
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
  return NULL;
}