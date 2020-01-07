#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "future.h"

typedef struct pair {
  uint64_t first;
  uint64_t second;
} pair_t;

void *multiply(void *data, size_t argsz __attribute__((unused)), size_t *size) {
  pair_t *pair = (pair_t *)(data);

  pair_t temp = {pair->first, pair->second};

  *size = sizeof(pair_t);
  pair->second = temp.second + 3;

  pair->first = (temp.first) * (temp.second);
  assert(pair->first >= temp.first);

  return pair;
}

int main() {
  thread_pool_t pool;
  if (thread_pool_init(&pool, 3) != 0) {
    return -1;
  }

  uint32_t n;
  scanf("%d", &n);

  if (n == 0) {
    printf("%d\n", 1);
  }
  else if (n == 1 || n == 2) {
    printf("%d\n", n);
  }
  else if (n == 3) {
    printf("%d\n", 6);
  }
  else if (n == 4) {
    printf("%d\n", 24);
  }
  else {
    future_t *future = malloc((n + 3) * sizeof(future_t));
    if (!future) {
      thread_pool_destroy(&pool);
      return -1;
    }

    callable_t callable1, callable2, callable3;

    pair_t pair1 = {1, 2};
    pair_t pair2 = {1, 3};
    pair_t pair3 = {1, 4};

    callable1.arg = &pair1;
    callable1.argsz = sizeof(pair_t);
    callable1.function = multiply;

    callable2.arg = &pair2;
    callable2.argsz = sizeof(pair_t);
    callable2.function = multiply;

    callable3.arg = &pair3;
    callable3.argsz = sizeof(pair_t);
    callable3.function = multiply;

    if (async(&pool, &future[2], callable1) != 0) {
      thread_pool_destroy(&pool);
      free(future);
      return -1;
    }

    if (async(&pool, &future[3], callable2) != 0) {
      thread_pool_destroy(&pool);
      free(future);
      return -1;
    }

    if (async(&pool, &future[4], callable3) != 0) {
      thread_pool_destroy(&pool);
      free(future);
      return -1;
    }

    uint32_t i = 5;
    uint32_t j = 6;
    uint32_t k = 7;

    while (i <= n || j <= n || k <= n) {

      if (map(&pool, &future[i], &future[i - 3], multiply) != 0) {
        thread_pool_destroy(&pool);
        free(future);
        return -1;
      }

      if (j <= n) {
        if (map(&pool, &future[j], &future[j - 3], multiply) != 0) {
          thread_pool_destroy(&pool);
          free(future);
          return -1;
        }
      }

      if (k <= n) {
        if (map(&pool, &future[k], &future[k - 3], multiply) != 0) {
          thread_pool_destroy(&pool);
          free(future);
          return -1;
        }
      }

      i+=3;
      j+=3;
      k+=3;
    }

    pair_t *result1 = await(&future[n]);
    pair_t *result2 = await(&future[n - 1]);
    pair_t *result3 = await(&future[n - 2]);

    if (!result1 || !result2 || !result3) {
      thread_pool_destroy(&pool);
      free(future);
      return -1;
    }

    printf("%" PRId64 "\n", result1->first * result2->first * result3->first);

    free(future);
  }

  thread_pool_destroy(&pool);

  return 0;
}