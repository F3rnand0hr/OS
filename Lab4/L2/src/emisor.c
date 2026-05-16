#define _GNU_SOURCE
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "functions.h"

int main() {
  mqd_t mq;
  mq = mq_open(QUEUE_NAME, O_WRONLY);
  if (mq == (mqd_t)-1) {
    perror("Error al abrir la cola");
    exit(1);
  }

  // Definimos una ráfaga extendida de 8 tareas
  Mensaje tareas[] = {
      // ID, Tipo, Nice, Carga, Core
      {1, 1, 19, 2600000, 0}, {2, 1, 0, 20000000, 0}, {3, 3, 5, 300000, 0},
      {4, 2, 10, 2514000, 1}, {5, 1, 0, 2000000, 1},  {6, 3, 19, 1500000, 0},
      {7, 2, 2, 4578500, 1},  {8, 1, 10, 2000000, 0}, {0, -1, 0, 0, 0}};

  int total_instrucciones = sizeof(tareas) / sizeof(Mensaje);

  printf("[EMITTER] Sending bunch of %d tasks..\n", total_instrucciones - 1);

  for (int i = 0; i < total_instrucciones; i++) {
    if (mq_send(mq, (char *)&tareas[i], sizeof(Mensaje), 1) == -1) {
      perror("Error sending message");
    } else {
      if (tareas[i].tipo_operacion == -1) {
        printf("[EMITTER] Shutdown signal sent.\n");
      } else {
        printf("[EMITTER] Task ID %d -> Core %d (Nice %d) sent.\n",
               tareas[i].id_tarea, tareas[i].core, tareas[i].prioridad);
        sleep(8);  // Mantener el delay para ver la progresión
      }
    }
  }

  mq_close(mq);
  return 0;
}