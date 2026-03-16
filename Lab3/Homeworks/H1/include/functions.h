#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

// Constant Variables
#define N 20
#define NUM_THREADS 4

// Matrix Declarations
extern int A[N][N];
extern int B[N][N];
extern int C[N][N];

// Function Declarations
void MatrixInit(void);
void calculateresult(int cmat[N][N]);
void* MatrixMultiplication(void* arg);

// Row Ranges Structure
typedef struct {
    int start_row;
    int end_row;
    } ThreadData;
    
    

#endif  /* INCLUDE_FUNCTIONS_H_ */