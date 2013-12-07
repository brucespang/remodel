#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>
#include <pthread.h>

typedef struct {
  void** buf;
  uint32_t head;
  uint32_t tail;
  uint32_t size;
  uint32_t max_size;
  pthread_mutex_t lock;
  pthread_cond_t cond;
} queue_t;

queue_t* queue_new(uint32_t initial_size);
void queue_free(queue_t* queue);
void queue_enqueue(queue_t* queue, void* data);
void* queue_dequeue(queue_t* queue);
uint32_t queue_size(queue_t* queue);

#endif  // QUEUE_H_
