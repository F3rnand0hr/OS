#include "../include/functions.h"

#include <stdio.h>
#include <stdlib.h>

// Implementación de la función del hilo

void *ThreadFunc(void *arg) {
  thread_data_t *data = (thread_data_t *)arg;

  // Kernel Gaussiano 5x5 (Valores enteros para evitar errores de redondeo
  // acumulados) El divisor común es la suma de todos los elementos: 273
  int kernel[5][5] = {{1, 4, 7, 4, 1},
                      {4, 16, 26, 16, 4},
                      {7, 26, 41, 26, 7},
                      {4, 16, 26, 16, 4},
                      {1, 4, 7, 4, 1}};
  float divisor = 273.0f;

  for (int i = data->start_row; i < data->end_row; i++) {
    for (int j = 0; j < data->width; j++) {
      // MANEJO DE BORDES: Para un kernel 5x5, necesitamos un margen de 2
      // píxeles. Si estamos en las primeras/últimas 2 filas o columnas,
      // copiamos el original.
      if (i < 2 || i >= data->height - 2 || j < 2 || j >= data->width - 2) {
        data->output[i * data->width + j] = data->input[i * data->width + j];
        continue;
      }

      float sum = 0.0f;
      // Recorremos el bloque de 5x5 alrededor del píxel (i, j)
      for (int ki = -2; ki <= 2; ki++) {
        for (int kj = -2; kj <= 2; kj++) {
          // Acceso a la matriz unidimensional de la imagen
          int pixel_val = data->input[(i + ki) * data->width + (j + kj)];
          // ki + 2 y kj + 2 mapean los índices [-2, 2] a [0, 4] para el kernel
          sum += (float)pixel_val * kernel[ki + 2][kj + 2];
        }
      }

      // Guardamos el resultado promediado
      data->output[i * data->width + j] = (unsigned char)(sum / divisor);
    }
  }
  return NULL;
}

// Funciones de I/O (se mantienen igual, pero asegúrate de que estén aquí)
unsigned char *LoadPgm(const char *filename, int *w, int *h) {
  FILE *f = fopen(filename, "rb");
  if (!f) {
    perror("Error opening image");
    exit(1);
  }
  char magic[3];
  int max_val;
  fscanf(f, "%s %d %d %d", magic, w, h, &max_val);
  fgetc(f);
  unsigned char *data = malloc((*w) * (*h));
  fread(data, sizeof(unsigned char), (*w) * (*h), f);
  fclose(f);
  return data;
}

void SavePgm(const char *filename, unsigned char *data, int w, int h) {
  FILE *f = fopen(filename, "wb");
  if (!f) return;
  fprintf(f, "P5\n%d %d\n255\n", w, h);
  fwrite(data, sizeof(unsigned char), w * h, f);
  fclose(f);
}