#include <pthread.h>
#include <unistd.h>
#include "../include/functions.h"

pthread_t threads[NUM_THREADS];
ThreadData data[NUM_THREADS];

int main(void) {
  MatrixInit();

  for (int i = 0; i < NUM_THREADS; i++){
    data[i].start_row = i * (N/NUM_THREADS); 
    data[i].end_row = data[i].start_row + (N/NUM_THREADS);
  }

  for(int i = 0; i < NUM_THREADS; i++){
    pthread_create(&threads[i], NULL, MatrixMultiplication, &data[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  calculateresult(C);

  return 0;
}