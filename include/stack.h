#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>
#include <stdbool.h>

// Standard stack data structure.
// NOTE: not thread safe.
// TODO: It would be great to call this 'stack_t' instead of 'stack', but OS X's stdlib.h is
// stupid and defines stack_T to their own fancy data structure.
struct stack {
  uint64_t max_size;
  uint64_t size;
  uint64_t top;
  void** stack;
};

struct stack* stack_new(uint64_t initial_size);
void stack_free(struct stack* stack);
bool stack_is_empty(struct stack* stack);
bool stack_contains(struct stack* stack, void* obj);
void* stack_pop(struct stack* stack);
uint64_t stack_size(struct stack* stack);

// insert item onto stack. automatically resize if necessary.
void stack_push(struct stack* stack, void* data);

#endif  // STACK_H_
