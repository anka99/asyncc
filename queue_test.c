#include <assert.h>
#include <stdio.h>

#include "queue.h"

int main() {
  queue *q = queue_init();

  int a[10] = {0,1,2,3,4,5,6,7,8,9};

  for (int i = 0; i < 10; i++) {
    assert(a[i] == i);
  }

  for (int i = 0; i < 10; i++) {
    queue_push(q, &(a[i]));
    assert(queue_size(q) == i + 1);
  }

  queue_delete(q, &(a[0]));
  assert(queue_size(q) == 9);

  queue_delete(q, &(a[9]));
  assert(queue_size(q) == 8);

  queue_delete(q, &(a[2]));
  assert(queue_size(q) == 7);

  for (int i = 0; i < 10; i++) {
    assert(a[i] == i);
  }

  for (int i = 0 ; i < 7; i++) {
    int *x = queue_pop(q);
    printf("%d\n", *x);
  }
  queue_destroy(q);
  return 0;
}