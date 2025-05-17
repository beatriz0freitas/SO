#ifndef CACHE_H
#define CACHE_H

#include "metaInformation.h"
#include <glib.h>

typedef struct Cache {
    GQueue *queue; // Queue to maintain the order of elements
    GHashTable *table; // Hash table for quick lookup
    int max_size; // Maximum number of elements in the cache
} Cache;

Cache *cache_new(int max_size);
void cache_free(Cache *cache);
MetaInformation *cache_get(Cache *cache, int key);
void cache_put(Cache *cache, int key, MetaInformation *value);
void cache_remove(Cache *cache, int key);

#endif
