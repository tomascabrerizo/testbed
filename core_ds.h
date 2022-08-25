#ifndef CORE_DS_H
#define CORE_DS_H

#include "core_type.h"

/* ------------------------CIRCULAR DOUBLE LINK LIST-----------------------------------------*/
  /* TODO: Make a circular double link list macros */
#define core_c_dll_init(p) ((dummy)->next = (dummy), (dummy)->prev = (dummy))
#define core_c_dll_push_back(dummy, v) ((v)->next = (dummy), \
                                        (v)->prev = (dummy)->prev, \
                                        (v)->prev->next = (v), \
                                        (v)->next->prev = (v))
#define core_c_dll_push_front(dummy, v) ((v)->prev = (dummy), \
                                         (v)->next = (dummy)->next, \
                                         (v)->prev->next = (v), \
                                         (v)->next->prev = (v))
#define core_c_dll_remove(v) ((v)->next->prev = (v)->prev, \
                              (v)->prev->next = (v)->next)

/* ---------------------------HASH MAP----------------------------------------------- */

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

/* ---------------------------STRECHED BUFFER----------------------------------------------- */

/* NOTE: Streched buffers (c dynamic array) */
/* TODO: This buffer are not tested yet */
typedef struct CoreBuffHeader {
  uint64_t size;
  uint64_t cap;
} CoreBuffHeader;

#define CORE_BUFFER_DEFAULT_CAP 8

#define core_buf_header(buf) (((CoreBuffHeader *)(buf))-1)
#define core_buf_size(buf) ((buf != 0) ? core_buf_header((buf))->size : 0)
#define core_buf_cap(buf) ((buf != 0) ? core_buf_header((buf))->cap : 0)
#define core_buf_fit(buf) (((core_buf_size((buf))+1) > core_buf_cap((buf))) ? (buf) = core_buf_grow((buf), sizeof(*(buf))) : 0)
#define core_buf_push(buf, value) (core_buf_fit(buf), (buf)[core_buf_header((buf))->size++] = value)

/* TODO: Remove stdlib.h from the header file */
#include <stdlib.h>
#include <stdio.h>

static inline void *core_buf_grow(void *buff, uint64_t element_size) {
  if(buff == NULL) {
    CoreBuffHeader *header = (CoreBuffHeader *)malloc(sizeof(CoreBuffHeader) + (CORE_BUFFER_DEFAULT_CAP*sizeof(element_size)));
    header->size = 0;
    header->cap = CORE_BUFFER_DEFAULT_CAP; 
    assert(header->cap > header->size);
    printf("init streched buffer\n");
    return (void *)(header + 1);
  } else {
    CoreBuffHeader *header = core_buf_header(buff);
    header->cap = header->cap * 2;
    header = realloc(header, sizeof(CoreBuffHeader) + header->cap*sizeof(element_size));
    assert(header->cap > header->size);
    printf("streched buffer grow\n");
    return (void *)(header + 1);
  }
}

static inline void core_buf_free(void *buff) {
  if(buff != NULL) {
    free(core_buf_header(buff));
  }
}

/* ---------------------------CUSTOM STRING----------------------------------------------- */

typedef struct CoreStr8 {
  uint8_t *data;
  uint64_t size;
} CoreStr8;

/* TODO: Implement string functions */
CoreStr8 core_str8(const char *cstr);
float core_str8_to_float(const char *cstr);
int core_str8_to_int(const char *cstr);
unsigned int core_str8_to_uint(const char *cstr);

/* ---------------------------HASH FUNCTIONS----------------------------------------------- */

/* NOTE: Mumur hash function https://sites.google.com/site/murmurhash/ */
static inline uint64_t core_hash64(const void *key, int len, unsigned int seed) {
	const uint64_t m = 0xc6a4a7935bd1e995ull;
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

#endif /* CORE_DS_H */
