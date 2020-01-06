#include "queue.h"

#include <stdlib.h>

queue *queue_init() {
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
  //printf("pushed\n");
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

    (q->size)--;
    free(temp);
    //printf("pop\n");
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
    (q->size)--;
  }

  q->head = NULL;
  q->tail = NULL;
}

int queue_size(queue *q) {
  if (!q) {
    return 0;
  }
  return q->size;
}

void queue_destroy(queue *q) {
  if (q) {
    queue_clear(q);
    free(q);
  }
}

void *queue_delete(queue *q, void *data) {
  if (q->size == 0 || (q->head == q->tail && q->head->data != data)) {
    return NULL;
  }
  if (q->head->data == data) {
    node *temp = q->head;
    if (q->tail == q->head) {
      q->tail = NULL;
    }
    q->head = q->head->next;
    void *result = temp->data;
    (q->size)--;
    free(temp);
    return result;
  }

  node *temp = NULL;
  node *temp_next = q->head;

  while (temp_next != NULL) {
    if (temp_next->data == data) {
      if (temp_next == q->tail) {
        q->tail = temp;
      }
      temp->next = temp_next->next;
      void *result = temp_next->data;
      (q->size)--;
      free(temp_next);
      return result;
    }
    temp = temp_next;
    temp_next = temp_next->next;
  }

  return NULL;
}