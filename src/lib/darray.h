#ifndef _darray
#define _darray

#include <stdlib.h>
#include <assert.h>
#include <lib/dbg.h>


/**
  A dynamic array is an array of `void **` pointers, so dynamic arrays can
  end up holding any data type.

  [
    0x000 => 0x111 => "Hello",
    0x001 => 0x222 => 123,
    0x002 => 0x333 => {.age=32, .name="Pat"},
  ]

*/
typedef struct {
  int end;
  int max;
  size_t element_size;
  size_t expand_rate;
  void **contents;
} DArray;

DArray * DArray_init(size_t element_size, size_t initial_max);

void DArray_destroy(DArray * array);

void DArray_clear(DArray * array);

int DArray_expand(DArray * array);

int DArray_contract(DArray * array);

int DArray_push(DArray * array, void * value);

void * DArray_pop(DArray * array);

void DArray_cleanup(DArray * array);

void DArray_qsort(DArray * array, int lo, int hi);
int DArray_partition(DArray * array, int lo, int hi);


#define DArray_last(A)  ((A)->contents[(A)->end - 1])
#define DArray_first(A) ((A)->contents[0])
#define DArray_end(A)   ((A)->end)
#define DArray_count(A) DArray_end(A)
#define DArray_max(A)   ((A)->max)
#define DArray_last_index(A) (DArray_end(A) - 1)

#define DEFAULT_EXPAND_RATE 300

static inline void DArray_set(DArray * array, int i, void * value) {
  check(i < array->max, "darray attempt to set past max");

  if (i > array->end) { array->end = i; }

  array->contents[i] = value;

error:
  return;
}

static inline void * DArray_get(DArray * array, int i) {
  check(i < array->max, "darray attempt to get past max");

  return array->contents[i];

error:
  return NULL;
}

static inline void * DArray_remove(DArray * array, int i) {
  check(array != NULL, "darray is NULL");
  check(i < array->max, "darray attempt to remove past max");

  void * el = array->contents[i];

  array->contents[i] = NULL;

  return el;

error:
  return NULL;
}

static inline void * DArray_create_element(DArray * array) {
  check(array->element_size > 0,
    "Can't use DArray_create_element on 0 size arrays.");

  return calloc(1, array->element_size);

error:
  return NULL;
}

#define DArray_free(E)  free((E))


#endif
