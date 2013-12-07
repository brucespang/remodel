#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "include/queue.h"

queue_t* queue_new(uint32_t max_size) {
  queue_t* queue = calloc(1, sizeof(queue_t));
  queue->buf = calloc(max_size, sizeof(void*));
  queue->max_size = max_size;
  queue->size = 0;
  pthread_mutex_init(&queue->lock, NULL);
  pthread_cond_init(&queue->cond, NULL);

  return queue;
}

void queue_free(queue_t* queue) {
  free(queue->buf);
  free(queue);
}

void* queue_dequeue_locked(queue_t* queue) {
  if (queue->head == queue->tail) {
    return NULL;
  }

  void* tmp = queue->buf[queue->head];
  queue->head = (queue->head + 1) % queue->max_size;
  queue->size--;

  return tmp;
}

void* queue_dequeue(queue_t* queue) {
  pthread_mutex_lock(&queue->lock);

  while (queue->head == queue->tail) {
    pthread_cond_wait(&queue->cond, &queue->lock);
  }

  void* res = queue_dequeue_locked(queue);
  pthread_mutex_unlock(&queue->lock);

  return res;
}

bool queue_enqueue_locked(queue_t* queue, void* item) {
  uint32_t next = (queue->tail + 1) % queue->max_size;
  if (next == queue->head) {
    return false;
  }

  queue->buf[queue->tail] = item;
  queue->tail = next;
  queue->size++;
  assert(queue->size <= queue->max_size);

  return true;
}

static void queue_grow(queue_t* queue, uint32_t new_size) {
  void** buf = calloc(new_size, sizeof(void*));
  uint32_t idx;
  for (idx = 0; queue->head != queue->tail; idx++) {
    buf[idx] = queue->buf[queue->head];
    queue->head = (queue->head + 1) % queue->max_size;
  }

  free(queue->buf);
  queue->buf = buf;

  queue->head = 0;
  queue->tail = idx;
  queue->max_size = new_size;
}

void queue_enqueue(queue_t* queue, void* val) {
  pthread_mutex_lock(&queue->lock);

  while(!queue_enqueue_locked(queue, val)) {
    queue_grow(queue, 2*queue->max_size+1);
  }

  pthread_cond_signal(&queue->cond);
  pthread_mutex_unlock(&queue->lock);
}

uint32_t queue_size(queue_t* queue) {
  return queue->size;
}
