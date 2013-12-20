#ifndef HT_H_
#define HT_H_

#include <ck_hs.h>
#include <ck_spinlock.h>
#include <stdint.h>

#define HT_ITERATOR_INITIALIZER CK_HS_ITERATOR_INITIALIZER

typedef struct {
	const void* key;
	size_t key_len;
	void* value;
} ht_entry_t;

typedef struct ht {
  ck_hs_t map;
  ck_spinlock_t lock;
  // we usually allocate memory for a key/value when inserting into a hash table.
  // we'd like to free that memory when we free the hash table. However, if we're
  // using ref counts, we may not know when the hash table is freed.
  // free_cb is called when the hash table is freed, which allows the program
  // to free any unused memory.
  void (*free_cb)(struct ht*);
} ht_t;
typedef ck_hs_iterator_t ht_iterator_t;

ht_t* ht_new(uint32_t capacity);
void* ht_get(ht_t* ht, const void* key, size_t key_len);
bool ht_put(ht_t* ht, const void* key, size_t key_len, void* value);
void* ht_remove(ht_t* ht, const void* key, size_t key_len);
ht_entry_t* ht_remove_entry(ht_t* ht, const void* key, size_t key_len);
void ht_iterator_init(ht_iterator_t* it);
bool ht_next(ht_t* ht, ht_iterator_t* it, ht_entry_t** entry);
void ht_free(ht_t* ht);
uint64_t ht_count(ht_t* ht);

#endif  // HT_H_
