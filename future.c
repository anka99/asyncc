#include "future.h"
#include <stdlib.h>
#include <assert.h>

typedef void *(*function_t)(void *);

/*
typedef struct future {
  void *result;
  pthread_mutex_t mutex;
} future_t;
*/

void function(void *args, size_t size) {
  future_t *future = (future_t *)(args);
  future->result = future->callable->function(future->callable->arg,
          future->callable->argsz, &(future->result_size));

  free(future->callable);
  pthread_mutex_unlock(&(future->mutex)); //TODO: error
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
  int err = 0;

  runnable_t runnable;

  //future = malloc(sizeof(future_t)); //TODO: error; może nie trzeba

  callable_t *callable_pointer = malloc(sizeof(callable_t)); //TODO: error

  callable_pointer->function = callable.function;
  callable_pointer->arg = callable.arg;
  callable_pointer->argsz = callable.argsz;

  future->callable = callable_pointer;
  pthread_mutex_init(&(future->mutex), 0);//TODO: error
  pthread_mutex_lock(&(future->mutex));

  runnable.arg = future;
  assert(sizeof(future_t) == sizeof(*future));

  runnable.argsz = sizeof(future_t); //TODO: być może zainicjalizowane

  runnable.function = function; //TODO: być może z &

  err = defer(pool, runnable);

  return err;
}

void function_map(void *args, size_t size) {
  future_t *future = (future_t *)(args);

  future_t *from = (future_t *)(future->callable->arg);

  pthread_mutex_lock(&(from->mutex));

  future->result = future->callable->function(future->callable->arg,
                                             future->callable->argsz,
                                             &(future->result_size));

  free(future->callable);
  pthread_mutex_unlock(&(future->mutex)); //TODO: error
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
  int err = 0;
  err = pthread_mutex_init(&(future->mutex), 0); //TODO: error
  err = pthread_mutex_lock(&(future->mutex)); //TODO: error

  callable_t *callable = malloc(sizeof(callable_t));
  callable->function = function;
  callable->arg = from;
  callable->argsz = sizeof(future_t);

  future->callable = callable;

  runnable_t runnable;
  runnable.arg = future;
  runnable.argsz = sizeof(*future);
  runnable.function = function_map; //TODO: być może z &

  err = defer(pool, runnable); //TODO: error
  return 0;
}

void *await(future_t *future) {
  pthread_mutex_lock(&(future->mutex)); //TODO: error
  return future->result;
}
