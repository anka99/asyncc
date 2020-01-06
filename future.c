#include "future.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "err.h"

typedef void *(*function_t)(void *);

void function(void *args, size_t size __attribute__((unused))) {
  future_t *future = (future_t *)(args);
  future->result = future->callable->function(future->callable->arg,
          future->callable->argsz, &(future->result_size));

  free(future->callable);
  if (sem_post(&(future->mutex))) {
    syserr("sem_post");
  } //TODO: check error
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
  int err = 0;

  runnable_t runnable;

  callable_t *callable_pointer = malloc(sizeof(callable_t));
  if (!callable_pointer) {
    return -1;
  }//TODO: check error

  callable_pointer->function = callable.function;
  callable_pointer->arg = callable.arg;
  callable_pointer->argsz = callable.argsz;

  future->callable = callable_pointer;
  err = sem_init(&(future->mutex), 0, 0);
  if (err != 0) {
    free(callable_pointer);
    return err;
  }//TODO: check error

  runnable.arg = future;
  assert(sizeof(future_t) == sizeof(*future));

  runnable.argsz = sizeof(future_t); //TODO: być może zainicjalizowane

  runnable.function = function;

  err = defer(pool, runnable);
  if (err != 0) {
    free(callable_pointer);
  }

  return err;
}

void function_map(void *args, size_t size __attribute__((unused))) {
  future_t *future = (future_t *)(args);

  future_t *from = (future_t *)(future->callable->arg);

  void *result = await(from);
  if (!result) {
    syserr("malloc");
  }

  future->result = future->callable->function(result, future->callable->argsz,
          &(future->result_size));

  free(future->callable);
  if (sem_post(&(future->mutex))) {
    syserr("sem_post");
  } //TODO: check error
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
  int err = 0;

  err = sem_init(&(future->mutex), 0, 0);
  if (err != 0) {
    return err;
  }//TODO: check error

  callable_t *callable = malloc(sizeof(callable_t));
  if (!callable) {
    return -1;
  }

  callable->function = function;
  callable->arg = from;
  callable->argsz = sizeof(future_t);

  future->callable = callable;

  runnable_t runnable;
  runnable.arg = future;
  runnable.argsz = sizeof(future_t);
  runnable.function = function_map;

  err = defer(pool, runnable);
  if (err != 0) {
    free(callable);
    return err;
  }//TODO: check error

  return 0;
}

void *await(future_t *future) {
  if (sem_wait(&(future->mutex))) {
    syserr("sem_wait");
  } //TODO: check error

  if (sem_destroy(&(future->mutex))) {
    syserr("sem_destroy");
  } //TODO: check error

  return future->result;
}