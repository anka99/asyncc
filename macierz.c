#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
  sem_wait(data->mutex);
  *(data->result) += data->value;
  sem_post(data->mutex);
  free(data);
}

int main(void) {
  thread_pool_t pool;
  thread_pool_init(&pool, THREADS_NUMBER);

  int k, n;
  scanf("%d %d", &k, &n);

  int *results = malloc(k * sizeof(int64_t));//TODO: error

  for (int i = 0; i < k; i++) {
    results[i] = 0;
  }
  sem_t *mutex_array = malloc(k * sizeof(sem_t));

  for (int i = 0; i < k; i++) {
    sem_init(&(mutex_array[i]), 0, 1); //TODO: error
  }

  int row = 0;

  for (int i = 0; i < k * n; i++) {
    row = i/n;
    int v, t;
    scanf("%d %d", &v, &t);

    data_t *data = malloc(sizeof(data_t)); //TODO: error
    data->mutex = &(mutex_array[row]);
    data->value = v;
    data->time = t;
    data->result = &(results[row]);

    runnable_t runnable;
    runnable.arg = data;
    runnable.argsz = sizeof(data_t);
    runnable.function = function;

    defer(&pool, runnable); //TODO: error
  }

  thread_pool_destroy(&pool);

  for (int i = 0; i < k; i++) {
    printf("%d\n", results[i]);
    sem_destroy(&(mutex_array[i]));
  }

  free(mutex_array);
  free(results);

  return 0;
}