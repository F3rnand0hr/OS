#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#include <sys/types.h>

// Nombre de la cola de mensajes
#define QUEUE_NAME "/scheduler_queue"

// Estructura del mensaje
typedef struct {
  int id_tarea;
  int tipo_operacion;  // 1: Primos, 2: Taylor, 3: Checksum
  int prioridad;       // nice value (0–19)
  int64_t carga;       // Cantidad de iteraciones para el bucle
  int core;
} Mensaje;

// Prototipos de las funciones matemáticas
void ExecutePrimes(int64_t n);
void ExecuteTaylorSeries(int64_t n);
void ExecuteChecksum(int64_t n);

#endif  //  INCLUDE_FUNCTIONS_H_