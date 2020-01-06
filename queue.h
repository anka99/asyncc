#include <stddef.h>

#ifndef ASYNC_QUEUE_H
#define ASYNC_QUEUE_H

typedef struct Node {
  void *data;
  struct Node *next;
} node;

typedef struct Queue {
  int size;
  node *head;
  node *tail;
} queue;

queue *queue_init();
int queue_push(queue *q, void *data);
void *queue_pop(queue *q);
void queue_clear(queue *q);
int queue_size(queue *q);
void queue_destroy(queue *q);
void *queue_delete(queue *q, void *data);

#endif //ASYNC_QUEUE_H
