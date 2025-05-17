#include "cache.h"
#include <stdlib.h>
#include <stdio.h>

Cache *cache_new(int max_size) {
    Cache *cache = g_new0(Cache, 1);
    cache->queue = g_queue_new();
    cache->table = g_hash_table_new_full(g_int_hash, g_int_equal, g_free, (GDestroyNotify)metaInformation_free);
    cache->max_size = max_size;
    return cache;
}

void cache_free(Cache *cache) {
    if (cache) {
        g_queue_free(cache->queue);
        g_hash_table_destroy(cache->table);
        g_free(cache);
    }
}

MetaInformation *cache_get(Cache *cache, int key) {
    int *key_ptr = g_new(int, 1);
    *key_ptr = key;
    MetaInformation *value = g_hash_table_lookup(cache->table, key_ptr);
    g_free(key_ptr);

    if (value) {
        // Move the accessed element to the end of the queue
        g_queue_remove(cache->queue, key_ptr);
        g_queue_push_tail(cache->queue, key_ptr);
    }

    return value;
}

void cache_put(Cache *cache, int key, MetaInformation *value) {
    int *key_ptr = g_new(int, 1);
    *key_ptr = key;

    if (g_hash_table_contains(cache->table, key_ptr)) {
        g_queue_remove(cache->queue, key_ptr);
    } else if (g_queue_get_length(cache->queue) >= cache->max_size) {
        // Remove the oldest element
        int *oldest_key = g_queue_pop_head(cache->queue);
        g_hash_table_remove(cache->table, oldest_key);
        g_free(oldest_key);
    }

    g_queue_push_tail(cache->queue, key_ptr);
    g_hash_table_insert(cache->table, key_ptr, value);
}
