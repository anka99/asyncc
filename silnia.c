#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "future.h"

void *multiply(void *data, size_t argsz, size_t *size) {

}

int main() {
  int n;
  scanf("%d", &n);
  assert(n >= 0);

  uint64_t result = 0;

  thread_pool_t pool;
  thread_pool_init(&pool, 3); //TODO: err

  future_t future;
  callable_t callable;
  callable.
  return 0;
}