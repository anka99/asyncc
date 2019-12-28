#include "queue.h"

#include <stdlib.h>
#include <assert.h>

queue *queue_init()
{
  queue *q = malloc(sizeof(queue));
  if (!q) {
    return NULL;
  }
  q->size = 0;
  q->head = NULL;
  q->tail = NULL;
  return q;
}

int queue_push(queue *q, void *data) {
  node *new_node = malloc(sizeof(node));

  if(new_node == NULL) {
    return -1;
  }

  new_node->data = malloc(sizeof(data));

  assert(new_node->data);

  new_node->next = NULL;

  new_node->data = data;

  if(q->size == 0) {
    q->head = new_node;
    q->tail = new_node;
  }
  else {
    q->tail->next = new_node;
    q->tail = new_node;
  }

  (q->size)++;
  return 0;
}

void *queue_pop(queue *q) {
  if(q->size > 0) {
    node *temp = q->head;
    void *data = q->head->data;

    if(q->size > 1) {
      q->head = q->head->next;
    }
    else {
      q->head = NULL;
      q->tail = NULL;
    }

    q->size--;
    free(temp);
    return data;
  }
  return NULL;
}

void queue_clear(queue *q) {
  node *temp;

  while(q->size > 0) {
    temp = q->head;
    q->head = temp->next;
    free(temp);
    q->size--;
  }

  q->head = NULL;
  q->tail = NULL;
}

int queue_size(queue *q) {
  return q->size;
}

void queue_destroy(queue *q) {
  if (q) {
    queue_clear(q);
    free(q);
  }
}