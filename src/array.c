#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "include/array.h"

// TODO: make this less shitty.

static void resize(array_t* arr, uint32_t new_size) {
  assert(new_size > arr->size);

  arr->arr = realloc(arr->arr, sizeof(void*)*new_size);
  assert(arr->arr);

  memset(arr->arr+arr->size, 0, sizeof(void*)*(new_size - arr->size));

  arr->size = new_size;
}

array_t* array_new() {
  array_t* arr = calloc(1, sizeof(array_t));
  assert(arr);

  arr->size = 2;
  arr->len = 0;

  arr->arr = calloc(arr->size, sizeof(void*));
  assert(arr->arr);

  return arr;
}

void array_set(array_t* arr, uint32_t idx, void* val) {
  if (idx >= arr->size) {
    resize(arr, 2*idx+1);
  }
  assert(idx < arr->size);

  if (!arr->arr[idx]) {
    arr->len++;
  }

  arr->arr[idx] = val;
}

void* array_get(array_t* arr, uint32_t idx) {
  if (idx >= arr->size) {
    return NULL;
  } else {
    return arr->arr[idx];
  }
}

void array_append(array_t* arr, void* val) {
  array_set(arr, arr->len, val);
}

void array_free(array_t* arr) {
  free(arr->arr);
  free(arr);
}
