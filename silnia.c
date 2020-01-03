#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <inttypes.h>
#include "future.h"

thread_pool_t pool;
pid_t pid;

typedef struct pair {
  uint64_t first; //(n - 1)!
  uint64_t second; //n
} pair_t;

typedef struct pool_pid {
  thread_pool_t pool;
  pid_t pid;
} pool_pid_t;

void *multiply(void *data, size_t argsz, size_t *size) {
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

void catch(int sig) {
  thread_pool_destroy(&pool);
  printf("sigint -> pool destroyed\n");
  kill(pid, SIGINT);
  strsignal(sig);
}

int main() {
  pid = getpid();
  if (thread_pool_init(&pool, 3) != 0) {
    return -1;
  } //TODO: err

  struct sigaction action;
  sigset_t block_mask;

  sigemptyset(&block_mask);
  sigaddset(&block_mask, SIGINT);

  action.sa_handler = catch;
  action.sa_mask = block_mask;
  action.sa_flags = SA_RESETHAND;

  if (sigaction(SIGINT, &action, 0) == -1) {
    thread_pool_destroy(&pool);
    return -1;
  }; //TODO: check error

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
      for (int j = n - 1; j >= 1; j--) {
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