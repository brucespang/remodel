#ifndef ARRAY_H_
#define ARRAY_H_

#include <stdint.h>

// Dynamic array. When something is inserted after the end of the array, the array expands
// to a size of 2(the new index) + 1.
// NOTE: not thread safe
typedef struct {
  // number of available spots in array
  uint32_t size;
  // number of items in the array
  uint32_t len;
  // array
  void** arr;
} array_t;

array_t* array_new(void);
void array_set(array_t* arr, uint32_t idx, void* val);
void array_append(array_t* arr, void* val);
void* array_get(array_t* arr, uint32_t idx);
void array_free(array_t* arr);

#endif  // ARRAY_H_
