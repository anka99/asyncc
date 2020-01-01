#include "threadpool.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

void *process(void *data) {
  assert(data);
  thread_pool_t *pool = (thread_pool_t *)(data);
  while (true) {
    sem_wait(&(pool->queue_mutex)); // TODO: error

    while (queue_size(pool->runnables) == 0) {
      sem_post(&(pool->queue_mutex)); //TODO: error
      sem_wait(&(pool->runnables_semaphore)); //TODO: error
      sem_wait(&(pool->queue_mutex)); // TODO: error

      if (pool->end && queue_size(pool->runnables) == 0) {
        sem_post(&(pool->queue_mutex));
        break;
      }
    }

    if (pool->end && queue_size(pool->runnables) == 0) {
      sem_post(&(pool->queue_mutex));
      break;
    }

    runnable_t *runnable = queue_pop(pool->runnables);

    sem_post(&(pool->queue_mutex)); //TODO: error

    (runnable->function)(runnable->arg, runnable->argsz);

    free(runnable);
  }
  return 0;
}

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
  pool->size = num_threads;
  pool->end = false;

  if (!(pool->runnables = queue_init())) {
    thread_pool_destroy(pool);
    return -1;
  }

  int result = sem_init(&(pool->queue_mutex), 0, 1);
  if (result != 0) {
    thread_pool_destroy(pool);
    return result;
  }

  result = sem_init(&(pool->runnables_semaphore), 0, 0);
  if (result != 0) {
    thread_pool_destroy(pool);
    return result;
  }

  pool->attr = malloc(sizeof(pthread_attr_t)); //TODO: error

  pool->threads = malloc(pool->size * sizeof(pthread_t *));

  result = pthread_attr_init(pool->attr);
  if (result != 0) {
    thread_pool_destroy(pool);
    return result;
  }

  result = pthread_attr_setdetachstate (pool->attr,PTHREAD_CREATE_JOINABLE);
  if (result != 0) {
    thread_pool_destroy(pool);
    return result;
  }

  for (size_t i = 0; i < pool->size; i++) {
    pool->threads[i] = malloc(sizeof(pthread_t)); //TODO: error
    result = pthread_create(pool->threads[i], pool->attr, process, pool);
    if (result != 0) {
      thread_pool_destroy(pool);
      return result;
    }
  }

  return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {
  int err = 0;
  pool->end = true;

  if (pool->threads) {
    for (size_t i = 0; i < pool->size; i++) {
      sem_post(&(pool->runnables_semaphore));
    }

    for (size_t i = 0; i < pool->size; i++) {
      if (pool->threads[i]) {
        err = pthread_join(*(pool->threads[i]), NULL);
        if (err != 0) {
          //TODO do sth; co jeśli wątek nie powstał?
        }
        free(pool->threads[i]);
      }
    }
    free(pool->threads);
  }

  err = pthread_attr_destroy(pool->attr);

  if (pool->attr) {
    free(pool->attr);
  }
  if (err != 0) {
    //TODO: do sth
  }


  queue_destroy(pool->runnables);

  sem_destroy(&(pool->queue_mutex));

  sem_destroy(&(pool->runnables_semaphore));
}

int defer(struct thread_pool *pool, runnable_t runnable) {
  assert(pool);
  assert(!pool->end);

  runnable_t *runnable_pointer = malloc(sizeof(runnable_t)); //TODO: error
  assert(runnable_pointer != NULL);
  runnable_pointer->arg = runnable.arg;
  runnable_pointer->argsz = runnable.argsz;
  runnable_pointer->function = runnable.function;

  sem_wait(&(pool->queue_mutex)); // TODO: error
  queue_push(pool->runnables, runnable_pointer); //TODO: error
  sem_post(&(pool->runnables_semaphore)); //TODO: error
  sem_post(&(pool->queue_mutex)); //TODO: error

  return 0;
}