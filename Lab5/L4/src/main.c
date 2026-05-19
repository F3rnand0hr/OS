#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "../include/functions.h"

// Definition of global variables
volatile sig_atomic_t real_seconds = 0;
volatile sig_atomic_t cpu_seconds = 0;
volatile sig_atomic_t actual_term = 0;
volatile sig_atomic_t terminate_program = 0;

static const char kMsgRealTimer[] = "[REAL TIMER] Tic...\n";
static const char kMsgSigint[] =
    "[SIGINT] Snapshot captured. Continuing calculation...\n";

static void SafeWrite(const char* msg, size_t len) {
  ssize_t written = write(STDOUT_FILENO, msg, len);
  (void)written;
}

static void HandleSigAlrm(int signo) {
  (void)signo;
  real_seconds++;
  SafeWrite(kMsgRealTimer, sizeof(kMsgRealTimer) - 1);
}

static void HandleSigVtAlrm(int signo) {
  (void)signo;
  cpu_seconds++;
}

static void HandleSigInt(int signo) {
  (void)signo;
  SafeWrite(kMsgSigint, sizeof(kMsgSigint) - 1);
}

static void HandleSigUsr1(int signo) {
  (void)signo;
  terminate_program = 1;
}

void SetupSignals(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sigemptyset(&sa.sa_mask);

  sa.sa_handler = HandleSigAlrm;
  sigaction(SIGALRM, &sa, NULL);

  sa.sa_handler = HandleSigVtAlrm;
  sigaction(SIGVTALRM, &sa, NULL);

  sa.sa_handler = HandleSigInt;
  sigaction(SIGINT, &sa, NULL);

  sa.sa_handler = HandleSigUsr1;
  sigaction(SIGUSR1, &sa, NULL);
}

// Safe handler (async-signal-safe)
void SetupTimers(void) {
  struct itimerval timer;
  memset(&timer, 0, sizeof(timer));
  timer.it_value.tv_sec = 1;
  timer.it_interval.tv_sec = 1;

  if (setitimer(ITIMER_REAL, &timer, NULL) != 0) {
    perror("setitimer ITIMER_REAL");
    exit(EXIT_FAILURE);
  }
  if (setitimer(ITIMER_VIRTUAL, &timer, NULL) != 0) {
    perror("setitimer ITIMER_VIRTUAL");
    exit(EXIT_FAILURE);
  }
}

// Set up signals and timers
static void StopTimers(void) {
  struct itimerval timer;
  memset(&timer, 0, sizeof(timer));
  setitimer(ITIMER_REAL, &timer, NULL);
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
}

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
  fflush(stdout);

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
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <matrix_size> <taylor_terms>\n", argv[0]);
    fprintf(stderr, "Example: %s 600 120\n", argv[0]);
    return EXIT_FAILURE;
  }

  int n = atoi(argv[1]);
  int k = atoi(argv[2]);
  if (n <= 0 || k <= 0) {
    fprintf(stderr, "Error: n and k must be positive integers.\n");
    return EXIT_FAILURE;
  }

  SetupSignals();
  SetupTimers();

  // Argument parsing
  CalculateExponential(n, k);

  // Call setup for signals, timers, and main function CalculateExponential

  // Print final report
  StopTimers();

  printf("\n--- FINAL REPORT ---\n");
  printf("Clock Seconds (Real): %d s\n", (int)real_seconds);
  printf("CPU Seconds (Virtual): %d s\n", (int)cpu_seconds);
  printf("Difference (Latency): %d s\n",
         (int)(real_seconds - cpu_seconds));

  return EXIT_SUCCESS;
}
