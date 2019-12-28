#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "queue.h"

typedef struct runnable {
  void (*function)(void *, size_t);
  void *arg;
  size_t argsz;
} runnable_t;

typedef struct thread_pool {
  bool end;
  pthread_attr_t *attr;
  pthread_mutex_t empty_queue_mutex; //mutex do czekania na pustą kolejkę
  size_t size; //liczba wątków
  queue *runnables; //kolejka zadań
  pthread_mutex_t queue_mutex; //ochrona kolejki
  sem_t runnables_semaphore; //semafor na zadania
  pthread_t **threads; //tablica wątków
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

#endif
