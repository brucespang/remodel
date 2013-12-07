#include <assert.h>
#include <stdlib.h>

#include "include/array.h"

int main() {
  array_t* arr = array_new();
  char* s = "a";

  array_set(arr, 0, s);
  assert(array_get(arr, 0) == s);
  assert(array_get(arr, 10) == NULL);
  assert(arr->len == 1);

  array_set(arr, 10, s);
  assert(array_get(arr, 10) == s);
  assert(array_get(arr, 0) == s);
  assert(arr->len == 2);

  array_free(arr);

  return 0;
}
