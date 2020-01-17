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
  size_t size; //number of threads
  queue *runnables; //tasks queue
  sem_t queue_mutex; //queue protection
  sem_t runnables_semaphore; //tasks semaphore
  pthread_t **threads; //array of threads
} thread_pool_t;

/*
 * Creates empty thread pool of given size.
 */
int thread_pool_init(thread_pool_t *pool, size_t pool_size);

/*
 * Waits for working threads to finish and destroys created thread pool. Frees
 * all memory allocated.
 */
void thread_pool_destroy(thread_pool_t *pool);

/*
 * Adds new task to thread pool.
 */
int defer(thread_pool_t *pool, runnable_t runnable);

#endif
