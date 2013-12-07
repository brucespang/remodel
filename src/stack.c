#include <assert.h>
#include <stdlib.h>

#include "include/stack.h"

struct stack* stack_new(uint32_t initial_size) {
  struct stack* stack = calloc(1, sizeof(struct stack));
  assert(stack);

  stack->stack = calloc(initial_size, sizeof(void*));
  assert(stack->stack);

  stack->max_size = initial_size;
  stack->size = 0;
  stack->top = 0;

  return stack;
}

void stack_free(struct stack* stack) {
  free(stack->stack);
  free(stack);
}

uint32_t stack_size(struct stack* stack) {
  return stack->size;
}

bool stack_is_empty(struct stack* stack) {
  return stack_size(stack) == 0;
}

bool stack_contains(struct stack* stack, void* obj) {
  for (uint32_t i = 0; i < stack->top; i++) {
    if (stack->stack[i] == obj) {
      return true;
    }
  }
  return false;
}

void* stack_pop(struct stack* stack) {
  if (stack_is_empty(stack)) {
    return NULL;
  }

  void* res = stack->stack[stack->top-1];
  stack->top--;
  stack->size--;

  return res;
}

static void resize(struct stack* stack, uint32_t new_len) {
  assert(new_len > stack->max_size);

  stack->stack = realloc(stack->stack, sizeof(void*)*new_len);
  assert(stack->stack);

  stack->max_size = new_len;

  assert(stack->top + 1 < stack->max_size);
}

void stack_push(struct stack* stack, void* val) {
  if (stack->top + 1 == stack->max_size) {
    resize(stack, 2*stack->max_size + 1);
  }

  stack->stack[stack->top] = val;
  stack->top++;
  stack->size++;
}
