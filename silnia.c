#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "future.h"

typedef struct pair {
  uint64_t first; //(n - 1)!
  uint64_t second; //n
} pair_t;

void *multiply(void *data, size_t argsz __attribute__((unused)), size_t *size) {
  pair_t *pair = (pair_t *)(data);

  pair_t *result = malloc(sizeof(pair_t)); //TODO: error
  if (!result) {
    exit(1);
  }

  *size = sizeof(pair_t);
  result->second = pair->second + 1;

  result->first = (pair->first) * (pair->second);
  assert(result->first >= pair->first);

  free(pair);
  return result;
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
  else {
    future_t **future = malloc(n * sizeof(void *));
    if (!future) {
      thread_pool_destroy(&pool);
      return -1;
    }

    future[1] = malloc(sizeof(future_t));
    if (!future[1]) {
      thread_pool_destroy(&pool);
      free(future);
      return -1;
    }

    callable_t callable;

    pair_t *pair = malloc(sizeof(pair_t));
    if (!pair) {
      thread_pool_destroy(&pool);
      free(future[1]);
      free(future);
      return -1;
    }

    pair->first = 1;
    pair->second = 2;
    callable.arg = pair;
    callable.argsz = sizeof(pair_t);
    callable.function = multiply;

    if (async(&pool, future[1], callable) != 0) {
      thread_pool_destroy(&pool);
      free(pair);
      free(future[1]);
      free(future);
      return -1;
    } //TODO: check err

    for(uint64_t i = 2; i < n; i++) {
      future[i] = malloc(sizeof(future_t));
      if (!future[i]) {
        thread_pool_destroy(&pool);
        free(pair);
        for (int j = i; j >= 1; j--) {
          free(future[j]);
        }
        free(future);
        return -1;
      }

      if (map(&pool, future[i], future[i - 1], multiply) != 0) {
        thread_pool_destroy(&pool);
        free(pair);
        for (int j = i; j >= 1; j--) {
          free(future[j]);
        }
        free(future);
        return -1;
      }
    }

    pair_t *result = await(future[n - 1]);
    if (!result) {
      thread_pool_destroy(&pool);
      free(pair);
      for (uint32_t j = n - 1; j >= 1; j--) {
        free(future[j]);
      }
      free(future);
      return -1;
    }//TODO: check error

    printf("%" PRId64 "\n", result->first);

    for (uint64_t i = 1; i < n; i++) {
      free(future[i]);
    }

    free(future);

    free(result);
  }

  thread_pool_destroy(&pool);

  return 0;
}

//void *multiply(void *data, size_t argsz __attribute__((unused)), size_t *size) {
//  pair_t *pair = (pair_t *)(data);
//
//  //pair_t *result = malloc(sizeof(pair_t)); //TODO: error
//  pair_t temp = {pair->first, pair->second};
//
//  *size = sizeof(pair_t);
//  pair->second = temp.second + 1;
//
//  pair->first = (temp.first) * (temp.second);
//  assert(pair->first >= temp.first);
//
//  //free(pair);
//  return pair;
//}
//
//int main() {
//  thread_pool_t pool;
//  if (thread_pool_init(&pool, 3) != 0) {
//    return -1;
//  }
//
//  uint32_t n;
//  scanf("%d", &n);
//
//  if (n == 0) {
//    printf("%d\n", 1);
//  }
//  else if (n == 1 || n == 2) {
//    printf("%d\n", n);
//  }
//  else if (n == 3) {
//    printf("%d\n", 6);
//  }
//  else if (n == 4) {
//    printf("%d\n", 24);
//  }
//  else {
//    future_t *future = malloc(n * sizeof(future_t));
//    if (!future) {
//      thread_pool_destroy(&pool);
//      return -1;
//    }
//
//    callable_t callable1, callable2, callable3;
//
//    pair_t pair1 = {1, 2};
//    pair_t pair2 = {1, 3};
//    pair_t pair3 = {1, 4};
//
//    callable1.arg = &pair1;
//    callable1.argsz = sizeof(pair_t);
//    callable1.function = multiply;
//
//    callable2.arg = &pair2;
//    callable2.argsz = sizeof(pair_t);
//    callable3.function = multiply;
//
//    callable2.arg = &pair3;
//    callable2.argsz = sizeof(pair_t);
//    callable3.function = multiply;
//
//    if (async(&pool, &future[1], callable1) != 0) {
//      thread_pool_destroy(&pool);
//      free(future);
//      return -1;
//    } //TODO: check err
//
//    if (async(&pool, &future[2], callable2) != 0) {
//      thread_pool_destroy(&pool);
//      free(future);
//      return -1;
//    }
//
//    if (async(&pool, &future[3], callable3) != 0) {
//      thread_pool_destroy(&pool);
//      free(future);
//      return -1;
//    }
//
//    uint32_t i = 4;
//    uint32_t j = 5;
//    uint32_t k = 6;
//
//    while (i < n || j < n || k < n) {
//
//      if (map(&pool, &future[i], &future[i - 3], multiply) != 0) {
//        thread_pool_destroy(&pool);
//        free(future);
//        return -1;
//      }
//
//      if (j < n && map(&pool, &future[j], &future[j - 3], multiply) != 0) {
//        thread_pool_destroy(&pool);
//        free(future);
//        return -1;
//      }
//
//      if (k < n && map(&pool, &future[k], &future[k - 3], multiply) != 0) {
//        thread_pool_destroy(&pool);
//        free(future);
//        return -1;
//      }
//      i++;
//      j++;
//      k++;
//    }
//
//    pair_t *result1 = await(&future[n - 1]);
//    pair_t *result2 = await(&future[n - 2]);
//    pair_t *result3 = await(&future[n - 3]);
//
//    if (!result1 || !result2 || !result3) {
//      thread_pool_destroy(&pool);
//      free(future);
//      return -1;
//    }//TODO: check error
//
//    printf("%" PRId64 "\n", result1->first * result2->first * result3->first);
//
//    free(future);
//  }
//
//  thread_pool_destroy(&pool);
//
//  return 0;
//}

