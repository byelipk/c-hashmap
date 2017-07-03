#undef NDEBUG
#include <stdint.h>
#include <hashmap/hashmap.h>
#include <lib/dbg.h>
#include <lib/bstrlib.h>

static int default_compare(void * a, void * b)
{
    return bstrcmp((bstring)a, (bstring)b);
}

/**
*   Simple Bob Jenkins's hash algorithm taken from the wikipedia
*   description.
*/
static uint32_t default_hash(void * a)
{
    size_t len = blength((bstring) a);
    char * key = bdata((bstring) a);

    uint32_t hash = 0;
    uint32_t i    = 0;

    for (hash = i = 0; i < len; ++i) {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 6);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

/**

    ALGORITHM:

    Allocate memory for map
    Set map->compare
    Set map->hash

    Allocate memory for map->buckets
    Set map->buckets
    Set map->buckets->end

    return map

*/
Hashmap * Hashmap_create(Hashmap_compare compare, Hashmap_hash hash)
{
    Hashmap * map = calloc(1, sizeof(Hashmap));
    check_mem(map);

    map->compare = compare == NULL ? default_compare : compare;
    map->hash    = hash == NULL ? default_hash : hash;

    // Build the buckets
    map->buckets = DArray_init(sizeof(DArray *), DEFAULT_NUMBER_OF_BUCKETS);

    // fake out expanding the array
    map->buckets->end = map->buckets->max;

    check_mem(map->buckets);

    return map;

error:
    if (map) {
        Hashmap_destroy(map);
    }

    return NULL;
}

/**

    ALGORITHM:

    For each bucket in map->buckets
      For each element in bucket
        free(element)
      free(bucket)
    free(map->buckets)
    free(map)
*/
void Hashmap_destroy(Hashmap * map)
{
    int i = 0;
    int j = 0;

    if (map) {
        if (map->buckets) {
            for (i = 0; i < DArray_count(map->buckets); i++) {
                DArray * bucket = DArray_get(map->buckets, i);
                if (bucket) {
                    for (j = 0; j < DArray_count(bucket); j++) {
                        free(DArray_get(bucket, i));
                    }
                    DArray_destroy(bucket);
                }
            }
            DArray_destroy(map->buckets);
        }
        free(map);
    }
}

/**

    ALGORITHM:

    Allocate memory for node
    Set node->key
    Set node->data
    Set node->hash

    return node

*/
static inline HashmapNode *Hashmap_node_create(int hash, void *key, void *data)
{
    HashmapNode * node = calloc(1, sizeof(HashmapNode));
    check_mem(node);

    node->key   = key;
    node->data = data;
    node->hash  = hash;

    return node;

error:
    return NULL;
}

/**

    ALGORITHM:

    Generate hash of key
    Use hash to identify bucket where value is stored
    If bucket does not exist
        Create new bucket
        Set key-value pair in new bucket

    return bucket

*/
static inline DArray * Hashmap_find_bucket(
    Hashmap * map, void * key, int create, uint32_t * hash_out)
{
    check(map != NULL, "Invalid map.");

    uint32_t hash = map->hash(key);
    int bucket_n  = hash % DEFAULT_NUMBER_OF_BUCKETS;
    check(bucket_n >= 0, "Invalid bucket found: %d", bucket_n);

    // Store it for the return so the caller can use it
    *hash_out = hash;

    DArray * bucket = DArray_get(map->buckets, bucket_n);

    if (!bucket && create) {
        // New bucket, set it up
        bucket = DArray_init(sizeof(void *), DEFAULT_NUMBER_OF_BUCKETS);
        check_mem(bucket);
        DArray_set(map->buckets, bucket_n, bucket);
    }

    return bucket;

error:
    return NULL;
}

/**

    ALGORITHM:

    Find bucket
    Allocate memory for new node
    Append node to bucket

    return 0

*/
int Hashmap_set(Hashmap * map, void * key, void * data)
{
    uint32_t hash = 0;
    DArray * bucket = Hashmap_find_bucket(map, key, 1, &hash);
    check(bucket, "Error: Can't create bucket.");

    HashmapNode * node = Hashmap_node_create(hash, key, data);
    check_mem(node);

    DArray_push(bucket, node);

    return 0;

error:
    return -1;
}

/**

    ALGORITHM:

    For each node in bucket
        Find node whose hash and key matches the one we're looking for

        Return index position of node

*/
static inline int Hashmap_get_node(
    Hashmap * map, uint32_t hash, DArray * bucket, void * key)
{
    int i = 0;

    for (i = 0; i < DArray_end(bucket); i++) {
        debug("TRY: %d", i);
        HashmapNode * node = DArray_get(bucket, i);
        if (node->hash == hash && map->compare(node->key, key) == 0) {
            return i;
        }
    }

    return -1;
}

/**

    ALGORITHM:

    Find bucket that contains the key we're looking for
    Find the node in the bucket

    Return data stored in the node

*/
void * Hashmap_get(Hashmap * map, void * key)
{
    uint32_t hash = 0;
    DArray * bucket = Hashmap_find_bucket(map, key, 0, &hash);

    if (!bucket) {
        return NULL;
    }

    int i = Hashmap_get_node(map, hash, bucket, key);
    if (i == -1) {
        return NULL;
    }

    HashmapNode * node = DArray_get(bucket, i);
    check(node != NULL,
        "Failed to get node from bucket when it should exist.");

    return node->data;

error:
    return NULL;
}

/**

    ALGORITHM:

    For each bucket in map->buckets
        For each node in bucket
            Apply callback function to node

*/
int Hashmap_traverse(Hashmap * map, Hashmap_traverse_cb traverse_cb)
{
    check(map != NULL, "Invalid map.");

    int i = 0;
    int j = 0;
    int rc = 0;

    for (i = 0; i < DArray_count(map->buckets); i++) {
        DArray * bucket = DArray_get(map->buckets, i);
        if (bucket) {
            for (j = 0; j < DArray_count(bucket); j++) {
                HashmapNode * node = DArray_get(bucket, j);
                if (node) {
                    rc = traverse_cb(node);
                    if (rc != 0) {
                        return rc;
                    }
                }
            }
        }
    }

    return 0;

error:
    return -1;
}

/**

    ALGORITHM:

    Find bucket that contains the key
    Find chosen node
    Store the chosen node's data
    Free the chosen node
    Delete node by swapping the chosen node with the last node

*/
void * Hashmap_delete(Hashmap * map, void * key)
{
    uint32_t hash = 0;
    DArray * bucket = Hashmap_find_bucket(map, key, 0, &hash);
    if (!bucket) {
        return NULL;
    }

    int i = Hashmap_get_node(map, hash, bucket, key);
    if (i == -1) {
        return NULL;
    }

    HashmapNode * node = DArray_get(bucket, i);
    void * data = node->data;
    free(node);

    HashmapNode * ending = DArray_pop(bucket);

    if (ending != node) {
        // Alright, looks like it's not the last one, so swap it
        DArray_set(bucket, i, ending);
    }

    return data;
}
