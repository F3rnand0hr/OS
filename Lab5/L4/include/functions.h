#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#include <signal.h>

// Global variables for communication with handlers (use of atomic types)
extern volatile sig_atomic_t real_seconds;
extern volatile sig_atomic_t cpu_seconds;
extern volatile sig_atomic_t actual_term;
extern volatile sig_atomic_t terminate_program;

// Function prototypes
void SetupSignals();
void SetupTimers();
void CalculateExponential(int n, int k);

// Auxiliary matrix functions
double** CreateMatrix(int n);
void FreeMatrix(double** matriz, int n);
void CopyMatrix(double** origen, double** destino, int n);
void MultiplyMatrix(double** A, double** B, double** C, int n);
void SumMatrix(double** A, double** B, int n);
void ScalarMatrix(double** A, double escalar, int n);

#endif  // INCLUDE_FUNCTIONS_H_