#include "threadpool.h"
#include <stdlib.h>
#include <assert.h>

void *process(void *data) {
  assert(data);
  thread_pool_t *pool = (thread_pool_t *)(data);
  while (true) {
    pthread_mutex_lock(&(pool->queue_mutex)); // TODO: error

    if (queue_size(pool->runnables) == 0) {
      pthread_mutex_unlock(&(pool->queue_mutex)); //TODO: error
      sem_wait(&(pool->runnables_semaphore)); //TODO: error
      pthread_mutex_lock(&(pool->queue_mutex)); // TODO: error
    }

    if (pool->end && queue_size(pool->runnables) == 0) {
      pthread_mutex_unlock(&(pool->queue_mutex));
      break;
    }
    runnable_t *runnable = queue_pop(pool->runnables);

    pthread_mutex_unlock(&(pool->queue_mutex)); //TODO: error

    runnable->function(runnable->arg, runnable->argsz);
  }
  return 0;
}

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
  if (!(pool = malloc(sizeof(thread_pool_t)))) {
    thread_pool_destroy(pool);
    return -1;
  }
  pool->size = num_threads;
  pool->end = false;
  pool->attr = NULL;

  if (!(pool->runnables = queue_init())) {
    thread_pool_destroy(pool);
    return -1;
  }

  int result = pthread_mutex_init(&(pool->queue_mutex), 0);
  if (result != 0) {
    thread_pool_destroy(pool);
    return result;
  }

  result = sem_init(&(pool->runnables_semaphore), 1, 0);
  if (result != 0) {
    thread_pool_destroy(pool);
    return result;
  }

  result = pthread_mutex_init(&(pool->empty_queue_mutex), 0);
  if (result != 0) {
    thread_pool_destroy(pool);
    return result;
  }

  result = pthread_attr_init(pool->attr);
  if (result != 0) {
    return result;
  }

  pool->runnables = queue_init();
  if (!(pool->runnables)) {
    thread_pool_destroy(pool);
    return -1;
  }

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
  //err = pthread_mutex_lock(&(pool->empty_queue_mutex));

  if (pool->threads) {
    for (size_t i = 0; i < pool->size; i++) {
      sem_post(&(pool->runnables_semaphore));
    }

    for (size_t i = 0; i < pool->size; i++) {
      err = pthread_join(*(pool->threads[i]), NULL);
      if (err != 0) {
        //TODO do sth; co jeśli wątek nie powstał?
      }
    }
    free(pool->threads);
  }

  if (pool->attr) {
    err = pthread_attr_destroy(pool->attr);
    if (err != 0) {
      //TODO: do sth
    }
  }

  queue_destroy(pool->runnables);

  pthread_mutex_destroy(&(pool->queue_mutex));

  pthread_mutex_destroy(&(pool->empty_queue_mutex));

  sem_destroy(&(pool->runnables_semaphore));
}

int defer(struct thread_pool *pool, runnable_t runnable) {
  assert(pool);
  assert(!pool->end);
  pthread_mutex_lock(&(pool->queue_mutex)); // TODO: error
  queue_push(pool->runnables, &runnable); //TODO :error
  sem_post(&(pool->runnables_semaphore)); //TODO: error
  pthread_mutex_unlock(&(pool->queue_mutex)); //TODO: error
  return 0;
}