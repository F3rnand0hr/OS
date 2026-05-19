#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "../include/functions.h"

// Definition of global variables

// Safe handler (async-signal-safe)

// Set up signals and timers

// --- Matrix logic ---

double** CreateMatrix(int n) {
  double** m = (double**)malloc(n * sizeof(double*));
  for (int i = 0; i < n; i++) {
    m[i] = (double*)calloc(n, sizeof(double));
  }
  return m;
}

void FreeMatrix(double** m, int n) {
  for (int i = 0; i < n; i++) free(m[i]);
  free(m);
}

void MultiplyMatrix(double** A, double** B, double** C, int n) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      C[i][j] = 0;
      for (int l = 0; l < n; l++) {
        C[i][j] += A[i][l] * B[l][j];
      }
    }
  }
}

void SumMatrix(double** A, double** B, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) A[i][j] += B[i][j];
}

void CalculateExponential(int n, int k) {
  printf("Calculate e^A (Taylor) to n=%d, k=%d...\n", n, k);

  double** A = CreateMatrix(n);
  double** ExpA = CreateMatrix(n);
  double** TerminoActual = CreateMatrix(n);
  double** Aux = CreateMatrix(n);

  for (int i = 0; i < n; i++) {
    A[i][i] = 0.1;
    ExpA[i][i] = 1.0;
    TerminoActual[i][i] = 1.0;
  }

  double factorial = 1.0;
  for (int i = 1; i <= k; i++) {
    actual_term = i;
    if (terminate_program) break;

    // Termino_i = (Termino_previo * A) / i
    MultiplyMatrix(TerminoActual, A, Aux, n);
    factorial = (double)i;

    for (int r = 0; r < n; r++) {
      for (int c = 0; c < n; c++) {
        TerminoActual[r][c] = Aux[r][c] / factorial;
      }
    }
    SumMatrix(ExpA, TerminoActual, n);
  }

  FreeMatrix(A, n);
  FreeMatrix(ExpA, n);
  FreeMatrix(TerminoActual, n);
  FreeMatrix(Aux, n);
}

int main(int argc, char* argv[]) {
  // Argument validation

  // Argument parsing

  // Call setup for signals, timers, and main function CalculateExponential

  // Print final report

  return 0;
}