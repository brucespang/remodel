#include <assert.h>

#include "include/queue.h"

static void basic_queue() {
  queue_t* queue = queue_new(4);
  assert(queue_size(queue) == 0);

  uint32_t vals[4];
  for (uint32_t i = 0; i < 4; i++) {
    vals[i] = i;
    queue_enqueue(queue, &vals[i]);
  }
  assert(queue_size(queue) == 4);

  for (int32_t i = 0; i < 4; i++) {
    int32_t* val = queue_dequeue(queue);
    assert(val);
    assert(i == *val);
  }
  assert(queue_size(queue) == 0);

  queue_free(queue);
}

static void resize_queue() {
  queue_t* queue = queue_new(2);
  assert(queue_size(queue) == 0);

  uint32_t vals[16];
  for (uint32_t i = 0; i < 16; i++) {
    vals[i] = i;
    queue_enqueue(queue, &vals[i]);
  }
  assert(queue_size(queue) == 16);

  for (int32_t i = 0; i < 16; i++) {
    int32_t* val = queue_dequeue(queue);
    assert(val);
    assert(i == *val);
  }

  queue_free(queue);
}

int main() {
  basic_queue();
  resize_queue();

  return 0;
}
