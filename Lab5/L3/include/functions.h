#ifndef INCLUDE_FUNCTIONS_H_
#define INCLUDE_FUNCTIONS_H_

#include <signal.h>
#include <time.h>

// El "Diccionario" de tareas
static const char *TASK_DICTIONARY[] = {
    "Clear memory cache",     //  Índice 0
    "Update routing tables",  //  Índice 1
    "Verify disk integrity",  //  Índice 2
    "Synchronize clocks",     //  Índice 3
    "Generate CPU report"     //  Índice 4
};

#define MAX_TAREAS_DICCIONARIO 5
#endif  //  INCLUDE_FUNCTIONS_H_
