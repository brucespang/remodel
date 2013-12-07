#include <stdlib.h>
#include <assert.h>

#include "include/ht.h"

static bool called;
static void cb(ht_t* ht) {
  (void) ht;
  called = true;
}

int main() {
  ht_t* ht = ht_new(4);
  ht->free_cb = cb;

  int v1 = 1;
  ht_put(ht, "test", 4, &v1);
  assert(ht_get(ht, "test", 4) == &v1);
  assert(ht_get(ht, "asdf", 4) == NULL);

  int v2 = 2;
  ht_put(ht, "a", 1, &v2);
  assert(ht_get(ht, "a", 1) == &v2);
  assert(ht_get(ht, "test", 4) == &v1);
  assert(ht_get(ht, "asdf", 4) == NULL);

  int v3 = 3;
  ht_put(ht, "ab", 2, &v3);
  assert(ht_get(ht, "a", 1) == &v2);
  assert(ht_get(ht, "test", 4) == &v1);
  assert(ht_get(ht, "ab", 2) == &v3);

  int v4 = 4;
  ht_put(ht, "abc", 3, &v4);
  assert(ht_get(ht, "a", 1) == &v2);
  assert(ht_get(ht, "test", 4) == &v1);
  assert(ht_get(ht, "ab", 2) == &v3);
  assert(ht_get(ht, "abc", 3) == &v4);

  ht_remove(ht, "abc", 3);
  assert(ht_get(ht, "a", 1) == &v2);
  assert(ht_get(ht, "test", 4) == &v1);
  assert(ht_get(ht, "ab", 2) == &v3);
  assert(ht_get(ht, "abc", 3) == NULL);

  ht_free(ht);
  assert(called);
}
