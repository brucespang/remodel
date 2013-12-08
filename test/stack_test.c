#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/stack.h"

static void basic_stack() {
  struct stack* stack = stack_new(4);
  uint32_t vals[4];

  for (uint32_t i = 0; i < 4; i++) {
    vals[i] = i;
    stack_push(stack, &vals[i]);
  }

  for (int32_t i = 3; i >= 0; i--) {
    int32_t* val = stack_pop(stack);
    assert(val);
    assert(i == *val);
  }

  stack_free(stack);
}

static void pop_empty_stack() {
  struct stack* stack = stack_new(4);
  assert(stack_pop(stack) == NULL);

  stack_free(stack);
}

static void resize_stack() {
  struct stack* stack = stack_new(2);

  uint32_t vals[16];

  for (uint32_t i = 0; i < 16; i++) {
    vals[i] = i;
    stack_push(stack, &vals[i]);
  }

  for (int32_t i = 15; i >= 0; i--) {
    int32_t* val = stack_pop(stack);
    assert(val);
    assert(i == *val);
  }

  stack_free(stack);
}

int main() {
  basic_stack();
  pop_empty_stack();
  resize_stack();

  return 0;
}
