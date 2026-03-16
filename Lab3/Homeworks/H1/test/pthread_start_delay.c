#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  void* (*start_routine)(void*);
  void* arg;
} WrapArg;

static void* start_wrapper(void* p) {
  WrapArg* w = (WrapArg*)p;

  // ESTE es el "sleep del test": hace que el hilo sea observable
  usleep(50000);  // 50 ms (puedes subir a 100000 si aún es muy rápido)

  void* (*real_fn)(void*) = w->start_routine;
  void* real_arg = w->arg;
  free(w);
  return real_fn(real_arg);
}

typedef int (*pthread_create_t)(pthread_t*, const pthread_attr_t*, void*(*)(void*), void*);

int pthread_create(pthread_t* thread, const pthread_attr_t* attr,
                   void* (*start_routine)(void*), void* arg) {
  static pthread_create_t real_create = NULL;
  if (!real_create) {
    real_create = (pthread_create_t)dlsym(RTLD_NEXT, "pthread_create");
    if (!real_create) _exit(127);
  }

  WrapArg* w = (WrapArg*)malloc(sizeof(WrapArg));
  if (!w) return real_create(thread, attr, start_routine, arg);

  w->start_routine = start_routine;
  w->arg = arg;

  return real_create(thread, attr, start_wrapper, (void*)w);
}