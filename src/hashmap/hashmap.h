#ifndef _hashmap
#define _hashmap

#include <stdint.h>
#include <lib/darray.h>

#define DEFAULT_NUMBER_OF_BUCKETS 100

typedef int (*Hashmap_compare) (void * a, void * b);
typedef uint32_t(*Hashmap_hash) (void * key);

/*
  A hashmap works by performing a hashing calculation on a keyword to
  produce an integer. It then uses that integer to find a bucket to get
  or set a key-value pair.
**/
typedef struct Hashmap {
    // It's a two-level mapping!
    // There are 100 buckets that make up the first level, and
    // key-value pairs go into these buckets based on the hash of the
    // key. Each bucket is a darray that contains HashmapNode structs
    // that are appended to the end as they are added.
    DArray * buckets;

    // Comparison function hashmap will use to find elements by their key
    Hashmap_compare compare;

    // The hashing function is responsible for taking a key, processing
    // it's contents, and producing a single uint32_t index number.
    Hashmap_hash hash;
} Hashmap;

typedef struct HashmapNode {
    void * key;
    void * data;
    uint32_t hash;
} HashmapNode;

typedef int (*Hashmap_traverse_cb) (HashmapNode * node);

Hashmap * Hashmap_create(Hashmap_compare compare, Hashmap_hash hash);
void Hashmap_destroy(Hashmap * map);

int Hashmap_set(Hashmap * map, void * key, void * data);
void * Hashmap_get(Hashmap * map, void * key);

int Hashmap_traverse(Hashmap * map, Hashmap_traverse_cb traverse_cb);

void * Hashmap_delete(Hashmap * map, void * key);

#endif
