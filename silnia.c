#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "future.h"

typedef struct pair {
  int first; //(n - 1)!
  int second; //n
} pair_t;

void *multiply(void *data, size_t argsz, size_t *size) {
  pair_t *pair = (pair_t *)(data);
  pair_t *result = malloc(sizeof(pair_t)); //TODO: error
  assert(pair);
  *size = sizeof(pair_t);
  result->second = pair->second + 1;

  result->first = (pair->first) * (pair->second);
  assert(result->first >= pair->first);

  free(pair);
  return result;
}

int main() {
  int n;
  scanf("%d", &n);
  assert(n >= 0);

  if (n == 0) {
    printf("%d", 1);
    return 0;
  }
  if (n == 1 || n == 2) {
    printf("%d", n);
    return 0;
  }


  thread_pool_t pool;
  thread_pool_init(&pool, 3); //TODO: err


  future_t **future = malloc(n * sizeof(void *));
  future[1] = malloc(sizeof(future_t));

  callable_t callable;

  pair_t *pair = malloc(sizeof(pair_t));
  pair->first = 1;
  pair->second = 2;
  callable.arg = pair;
  callable.argsz = sizeof(pair_t);
  callable.function = multiply;

  async(&pool, future[1], callable); //TODO: err

  for(int i = 2; i < n; i++) {
    future[i] = malloc(sizeof(future_t));
    map(&pool, future[i], future[i - 1], multiply);
  }

  pair_t *result = await(future[n - 1]); //TODO: error

  printf("%d", result->first);

  thread_pool_destroy(&pool);

  for (int i = 1; i < n; i++) {
    free(future[i]);
  }

  free(future);

  free(result);

  return 0;
}
