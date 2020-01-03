#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//#include <zconf.h>

#include "threadpool.h"

#define THREADS_NUMBER 4

typedef struct data {
  int *result;
  sem_t *mutex;
  uint64_t time;
  int value;
} data_t;

void function(void *args, size_t size) {

  data_t *data = (data_t *) (args);
  usleep(1000 * (data->time));

  if (sem_wait(data->mutex)) {
    exit(1);
  }

  *(data->result) += data->value;
  if (sem_post(data->mutex)) {
    exit(1);
  }

  free(data);
}

int main(void) {
  thread_pool_t pool;
  thread_pool_init(&pool, THREADS_NUMBER);

  int k, n;
  scanf("%d %d", &k, &n);

  int *results = malloc(k * sizeof(int64_t));
  if (!results) {
    thread_pool_destroy(&pool);
    return -1;
  }//TODO: error

  for (int i = 0; i < k; i++) {
    results[i] = 0;
  }

  sem_t *mutex_array = malloc(k * sizeof(sem_t));
  if (!mutex_array) {
    thread_pool_destroy(&pool);
    free(results);
    return -1;
  }

  for (int i = 0; i < k; i++) {
    if (sem_init(&(mutex_array[i]), 0, 1)) {
      thread_pool_destroy(&pool);
      free(results);
      return -1;
    } //TODO: check error
  }

  int row = 0;

  for (int i = 0; i < k * n; i++) {
    row = i/n;
    int v, t;
    scanf("%d %d", &v, &t);

    data_t *data = malloc(sizeof(data_t)); //TODO: error
    if (!data) {
      thread_pool_destroy(&pool);
      free(results);
      return -1;
    }

    data->mutex = &(mutex_array[row]);
    data->value = v;
    data->time = t;
    data->result = &(results[row]);

    runnable_t runnable;
    runnable.arg = data;
    runnable.argsz = sizeof(data_t);
    runnable.function = function;

    if (defer(&pool, runnable) != 0) {
      free(data);
      thread_pool_destroy(&pool);
      free(results);
      return -1;
    } //TODO: check error
  }

  thread_pool_destroy(&pool);

  for (int i = 0; i < k; i++) {
    printf("%d\n", results[i]);
    if (sem_destroy(&(mutex_array[i]))) {
      thread_pool_destroy(&pool);
      free(results);
      return -1;
    } //TODO: check error
  }

  free(mutex_array);
  free(results);

  return 0;
}