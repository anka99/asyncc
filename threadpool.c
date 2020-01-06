#include "threadpool.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include "err.h"
//#include <sys/types.h>
#include <unistd.h>

static queue *working_pools = NULL;

static sem_t working_pools_mutex;

static void lib_init (void) __attribute__ ((constructor));

static void lib_destroy (void) __attribute__ ((destructor));

static thread_pool_t *delete_from_working_pools(thread_pool_t *pool) {
  if (sem_wait(&working_pools_mutex)) {
    syserr("sem_wait");
  }
  thread_pool_t *result = (thread_pool_t *) queue_delete(working_pools, pool);

  if (sem_post(&working_pools_mutex)) {
    syserr("sem_wait");
  }
  return result;
}

static int push_to_working_pools(thread_pool_t *pool) {
  if (sem_wait(&working_pools_mutex)) {
    syserr("sem_wait");
  }
  int result = queue_push(working_pools, pool);

  if (sem_post(&working_pools_mutex)) {
    syserr("sem_wait");
  }
  return result;
}

//void syserr(const char *fmt, ...) {
//  va_list fmt_args;
//
//  fprintf(stderr, "ERROR: ");
//  va_start(fmt_args, fmt);
//  vfprintf(stderr, fmt, fmt_args);
//  va_end (fmt_args);
//  fprintf(stderr," (%d; %s)\n", errno, strerror(errno));
//  exit(1);
//}

void *process(void *data) {
  assert(data);
  thread_pool_t *pool = (thread_pool_t *)(data);

  while (true) {
    if (sem_wait(&(pool->queue_mutex))) syserr("sem_wait");// TODO: check error

    while (queue_size(pool->runnables) == 0) {
      if (sem_post(&(pool->queue_mutex))) syserr("sem_post");//TODO: check error
      if (sem_wait(&(pool->runnables_semaphore))) syserr("sem_wait"); //TODO: check error
      if (sem_wait(&(pool->queue_mutex))) syserr("sem_wait"); // TODO: check error

      if (pool->end && queue_size(pool->runnables) == 0) {
        if (sem_post(&(pool->queue_mutex))) syserr("sem_post");
        break;
      }
    }

    if (pool->end && queue_size(pool->runnables) == 0) {
      if (sem_post(&(pool->queue_mutex)))syserr("sem_post");
      break;
    }

    runnable_t *runnable = queue_pop(pool->runnables);

    if (sem_post(&(pool->queue_mutex))) syserr("sem_post"); //TODO: check error

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

  if (!(pool->attr = malloc(sizeof(pthread_attr_t)))) {
    thread_pool_destroy(pool);
    return result;
  } //TODO: check error

  if (!(pool->threads = malloc(pool->size * sizeof(pthread_t *)))) {
    thread_pool_destroy(pool);
    return result;
  } //TODO: check error

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

  return push_to_working_pools(pool);
}

static void raw_destroy(thread_pool_t *pool) {
  pool->end = true;

  if (pool->threads) {
    for (size_t i = 0; i < pool->size; i++) {
      if (sem_post(&(pool->runnables_semaphore))) {
        syserr("sem_post");
      } //TODO: check error
    }

    for (size_t i = 0; i < pool->size; i++) {
      if (pool->threads[i]) {
        if (pthread_join(*(pool->threads[i]), NULL)) {
          syserr("pthread_join");
        }
        free(pool->threads[i]);
      }
    }
    free(pool->threads);
  }

  if (pthread_attr_destroy(pool->attr)) {
    syserr("pthread_attr_destroy");
  }

  if (pool->attr) {
    free(pool->attr);
  }

  queue_destroy(pool->runnables);

  if (sem_destroy(&(pool->queue_mutex))) {
    syserr("sem_destroy");
  }

  if (sem_destroy(&(pool->runnables_semaphore))) {
    syserr("sem_destroy");
  }
}

void thread_pool_destroy(struct thread_pool *pool) {
  raw_destroy(pool);
  delete_from_working_pools(pool);
}

int defer(struct thread_pool *pool, runnable_t runnable) {
  assert(pool);
  if (pool->end) {
    return -1;
  }
  int result = 0;


  runnable_t *runnable_pointer = malloc(sizeof(runnable_t));
  if (!runnable_pointer) {
    return -1;
  }//TODO: check error
  runnable_pointer->arg = runnable.arg;
  runnable_pointer->argsz = runnable.argsz;
  runnable_pointer->function = runnable.function;

  result = sem_wait(&(pool->queue_mutex));
  if (result != 0) {
    free(runnable_pointer);
    return result;
  }

  result = queue_push(pool->runnables, runnable_pointer);
  if (result != 0) {
    free(runnable_pointer);
    return result;
  } //TODO: check error

  result = sem_post(&(pool->runnables_semaphore));
  if (result != 0) {
    free(runnable_pointer);
    return result;
  }//TODO: check error

  result = sem_post(&(pool->queue_mutex));
  if (result != 0) {
    free(runnable_pointer);
    return result;
  }//TODO: check error

  return 0;
}

void catch(int sig) {
  if (sem_wait(&working_pools_mutex)) {
    syserr("sem init");
  }
  while (queue_size(working_pools) > 0) {
    thread_pool_t *pool = queue_pop(working_pools);
    raw_destroy(pool);
    printf("sigint -> pool destroyed\n");
    strsignal(sig);
  }
  if (sem_post(&working_pools_mutex)) {
    syserr("sem post");
  }
}

void lib_init (void) {
  working_pools = queue_init();

  if (sem_init(&working_pools_mutex, 0, 1) != 0) {
    syserr("sem init");
  }

  struct sigaction action;
  sigset_t block_mask;

  sigemptyset(&block_mask);
  sigaddset(&block_mask, SIGINT);

  action.sa_handler = catch;
  action.sa_mask = block_mask;
  action.sa_flags = 0; //SA_RESETHAND;

  if (sigaction(SIGINT, &action, 0) == -1) {
    //TODO: error
  }
}

void lib_destroy (void) {
  queue_destroy(working_pools);
  if (sem_destroy(&working_pools_mutex)) {
    syserr("sem destroy");
  }
}