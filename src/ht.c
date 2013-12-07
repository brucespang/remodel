#include <ck_malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "include/ht.h"
#include "include/murmurhash.h"

#define SEED 0

void* ht_malloc(size_t r) {
  return malloc(r);
}

static void* ht_realloc(void *r, size_t a, size_t b, bool d) {
	(void)a;
	(void)d;

	return realloc(r, b);
}

void ht_mfree(void *p, size_t b, bool r) {
  (void)b;
  (void)r;

  free(p);
}

static struct ck_malloc ck_malloc = {
  .malloc = ht_malloc,
  .realloc = ht_realloc,
  .free = ht_mfree
};

bool ht_compare(const void* x, const void* y) {
  const ht_entry_t* a = x;
  const ht_entry_t* b = y;
  return a->key_len == b->key_len && memcmp(a->key, b->key, a->key_len) == 0;
}

unsigned long ht_hash(const void* e, unsigned long seed) {
  const ht_entry_t* entry = e;
  uint64_t h[2];
  MurmurHash3_x64_128(entry->key, entry->key_len, seed, h);
  return h[0];
}

ht_t* ht_new(unsigned long capacity) {
  ht_t* ht = calloc(1, sizeof(ht_t));
  assert(ht);

  ck_spinlock_init(&ht->lock);
  assert(ck_hs_init(&ht->map, CK_HS_MODE_DIRECT | CK_HS_MODE_SPMC,
                    ht_hash, ht_compare, &ck_malloc,
                    2*capacity, SEED));

  ht_ref_add(ht);

  return ht;
}

void ht_free(ht_t* ht) {
  if (ht->free_cb) {
    ht->free_cb(ht);
  }

  ht_entry_t* entry;
  ht_iterator_t iter = HT_ITERATOR_INITIALIZER;
  while (ht_next(ht, &iter, &entry) == true) {
    free(entry);
  }

  ck_hs_destroy(&ht->map);
  free(ht);
}

void ht_ref_add(ht_t* ht) {
  ph_refcnt_add(&ht->ref);
}

void ht_ref_del(ht_t* ht) {
  if (!ph_refcnt_del(&ht->ref)) {
    return;
  }

  ht_free(ht);
}

void* ht_get(ht_t* ht, const void* key, size_t key_len) {
  ht_entry_t entry = {
    .key = key,
    .key_len = key_len,
    .value = NULL
  };

  ht_entry_t* e = ck_hs_get(&ht->map, CK_HS_HASH(&ht->map, ht_hash, &entry), &entry);
  if (e) {
    return e->value;
  } else {
    return NULL;
  }
}

bool ht_put(ht_t* ht, const void* key, size_t key_len, void* value) {
  ht_entry_t* entry = malloc(sizeof(ht_entry_t));
  assert(entry);
  entry->key = key;
  entry->key_len = key_len;
  entry->value = value;

  ck_spinlock_lock(&ht->lock);
  bool res = ck_hs_put(&ht->map, CK_HS_HASH(&ht->map, ht_hash, entry), entry);
  if (res) {
    assert(ht_get(ht, key, key_len) == value);
  }
  ck_spinlock_unlock(&ht->lock);

  if (!res)
    free(entry);

  return res;
}

ht_entry_t* ht_remove_entry(ht_t* ht, const void* key, size_t key_len) {
  ht_entry_t entry = {
    .key = key,
    .key_len = key_len,
    .value = NULL
  };

  ck_spinlock_lock(&ht->lock);
  ht_entry_t* e = ck_hs_remove(&ht->map, CK_HS_HASH(&ht->map, ht_hash, &entry), &entry);
  ck_spinlock_unlock(&ht->lock);

  return e;
}

void* ht_remove(ht_t* ht, const void* key, size_t key_len) {
  ht_entry_t* e = ht_remove_entry(ht, key, key_len);
  if (e) {
    void* val = e->value;
    free(e);
    return val;
  } else {
    return NULL;
  }
}

void ht_iterator_init(ht_iterator_t* iter) {
  ck_hs_iterator_init(iter);
}

bool ht_next(ht_t* ht, ht_iterator_t* iter, ht_entry_t** entry) {
  return ck_hs_next(&ht->map, iter, (void**)entry);
}

uint32_t ht_count(ht_t* ht) {
  return ck_hs_count(&ht->map);
}
