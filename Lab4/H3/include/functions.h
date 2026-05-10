#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#include <pthread.h>

// Estructura para pasar datos a los hilos
typedef struct {
  unsigned char *input;
  unsigned char *output;
  int width;
  int height;
  int start_row;
  int end_row;
} thread_data_t;

// --- PROTOTIPOS ---

// Esta es la función que ejecutarán los hilos (definida en functions.c)
void *ThreadFunc(void *arg);

// Funciones de utilidad para PGM (definidas en functions.c)
unsigned char *LoadPgm(const char *filename, int *w, int *h);
void SavePgm(const char *filename, unsigned char *data, int w, int h);

#endif  //  INCLUDE_FUNCTIONS_H_