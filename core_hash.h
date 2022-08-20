#ifndef CORE_HASH_H
#define CORE_HASH_H

#include "core_type.h"

/* NOTE: Mumur hash function https://sites.google.com/site/murmurhash/ */
static inline uint64_t core_hash64(const void *key, int len, unsigned int seed) {
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;
	uint64_t h = seed ^ (len * m);
	const uint64_t * data = (const uint64_t *)key;
	const uint64_t * end = data + (len/8);
	while(data != end) {
		uint64_t k = *data++;
		k *= m; 
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	const unsigned char * data2 = (const unsigned char*)data;
#if 1
  int t = len & 7;
  if(t == 7) h ^= ((uint64_t)data2[6]) << 48;
  if(t == 6) h ^= ((uint64_t)data2[5]) << 40;
  if(t == 5) h ^= ((uint64_t)data2[4]) << 32;
  if(t == 4) h ^= ((uint64_t)data2[3]) << 24;
  if(t == 3) h ^= ((uint64_t)data2[2]) << 16;
  if(t == 2) h ^= ((uint64_t)data2[1]) << 8;
  if(t == 1) h ^= ((uint64_t)data2[0]);
#else
  switch(len & 7) {
    case 7: h ^= ((uint64_t)data2[6]) << 48;
    case 6: h ^= ((uint64_t)data2[5]) << 40;
    case 5: h ^= ((uint64_t)data2[4]) << 32;
    case 4: h ^= ((uint64_t)data2[3]) << 24;
    case 3: h ^= ((uint64_t)data2[2]) << 16;
    case 2: h ^= ((uint64_t)data2[1]) << 8;
    case 1: h ^= ((uint64_t)data2[0]);
            h *= m;
	};
#endif
  h *= m;
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
	return h;
} 

#define CORE_MAP_SEED 0
#define CORE_MAP_ENTRY_EMPTY 0
#define CORE_MAP_ENTRY_TOMBSTONE 1 /* TODO: Acully write the code to remove items */

/* TODO: Make this hash table be able to delete things */
typedef struct CoreMapEntry {
  void *key;
  void *value;
  uint64_t hash;
} CoreMapEntry;

typedef struct CoreMap {
  CoreMapEntry *entries;
  uint64_t size;
  uint64_t capacity;
} CoreMap;


CoreMap *core_map_create(void);
void core_map_destroy(CoreMap *map);

void core_map_add(CoreMap *map, void *key, void *value);
void *core_map_get(CoreMap *map, void *key);

void core_map_add_hash(CoreMap *map, void *key, void *value, uint64_t hash);
void *core_map_get_hash(CoreMap *map, void *key, uint64_t hash);

#endif /* CORE_HASH_H */
