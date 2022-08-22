#include "core_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

CoreMap *core_map_create(void) {
  CoreMap *map = (CoreMap *)malloc(sizeof(*map));
  memset(map, 0, sizeof(*map)); 
  return map;
}

void core_map_destroy(CoreMap *map) {
  if(map->entries) free(map->entries);
  free(map);
}

static void core_map_grow(CoreMap *map, uint64_t size) {
  uint64_t alloc_size = size * sizeof(CoreMapEntry);
  CoreMapEntry *new_entries = (CoreMapEntry *)malloc(alloc_size);
  memset(new_entries, 0, alloc_size);
  for(uint64_t i = 0; i < map->capacity; ++i) {
    CoreMapEntry *entry = map->entries + i;
    if(entry->key != CORE_MAP_ENTRY_EMPTY) {
      CoreMapEntry *new_entry = new_entries + (entry->hash & (size-1));
      *new_entry = *entry;
    }
  }
  if(map->entries) free(map->entries);
  map->entries = new_entries;
  map->capacity = size;
}

void core_map_add_hash(CoreMap *map, void *key, void *value, uint64_t hash) {
  assert(map);
  assert(key); /* NOTE: 0 key is use as empty slot */
  if((float)map->size >= (float)map->capacity*0.6f) {
    core_map_grow(map, CORE_MAX(16, map->capacity*2));
  }
  assert(CORE_IS_POWER_OF_TWO(map->capacity));
  uint64_t index = hash & (map->capacity-1);
  for(;;) {
    CoreMapEntry *entry = map->entries + index;
    if(entry->key == CORE_MAP_ENTRY_EMPTY) {
      entry->key = key;
      entry->value = value;
      entry->hash = hash;
      ++map->size; /* NOTE: Place for new entry found */
      return;
    } else if(entry->key == key) {
      entry->value = value;
      return;
    }
    ++index;
    index = index & (map->capacity - 1);
    /*if(index >= map->capacity) index = 0;*/
  }
}

/* TODO: This function is still not tested */
void *core_map_get_hash(CoreMap *map, void *key, uint64_t hash) {
  assert(map);
  assert(key);
  if(map->size == 0) {
    return 0;
  }
  assert(CORE_IS_POWER_OF_TWO(map->capacity));
  uint64_t index = hash & (map->capacity-1);
  for(;;) {
    CoreMapEntry *entry = map->entries + index;
    if(entry->key == key) {
      return entry->value;
    } else if(entry->key == CORE_MAP_ENTRY_EMPTY) {
      return 0;
    }
    ++index;
    index = index & (map->capacity - 1);
    /*if(index >= map->capacity) index = 0;*/
  }
}

void core_map_add(CoreMap *map, void *key, void *value) {
  assert(map);
  assert(key); /* NOTE: 0 key is use as empty slot */
  core_map_add_hash(map, key, value, core_hash64(&key, 8, CORE_MAP_SEED));
}

void *core_map_get(CoreMap *map, void *key) {
  assert(map);
  assert(key); /* NOTE: 0 key is use as empty slot */
  return core_map_get_hash(map, key, core_hash64(&key, 8, CORE_MAP_SEED));
}
