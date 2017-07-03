#include <lib/darray.h>
#include <assert.h>
#include <math.h>

DArray * DArray_init(size_t element_size, size_t initial_max) {
  DArray * array = malloc(sizeof(DArray));
  check_mem(array);

  array->max = initial_max;
  check(array->max > 0, "You must set an initial max greater than 0.");

  array->contents = calloc(initial_max, sizeof(void *));
  check_mem(array->contents);

  array->end          = 0;
  array->element_size = element_size;
  array->expand_rate  = DEFAULT_EXPAND_RATE;

  return array;

error:
  if (array) {
    DArray_free(array);
  }

  return NULL;
}

static inline int DArray_resize(DArray * array, size_t newsize) {
  array->max = newsize;
  check(array->max > 0, "The newsize must be greater than 0.");

  // See: man realloc
  void * contents = realloc(array->contents, array->max * sizeof(void *));

  check_mem(contents);

  array->contents = contents;

  return 0;

error:
  return -1;
}

int DArray_expand(DArray * array) {
  size_t oldmax = array->max;
  int did_resize_array = DArray_resize(array, array->max + array->expand_rate);

  check(did_resize_array == 0,
    "Failed to expand array to new size: %d",
    array->max + (int)array->expand_rate);

  memset(array->contents + oldmax, 0, array->expand_rate + 1);

  return 0;

error:
  return -1;
}

int DArray_contract(DArray * array) {
  int newsize = array->end < (int)array->expand_rate ?
    (int)array->expand_rate : array->end;

  return DArray_resize(array, newsize + 1);
}

int DArray_push(DArray * array, void * value) {
  array->contents[array->end] = value;
  array->end++;

  if (DArray_end(array) >= DArray_max(array)) {
    return DArray_expand(array);
  }
  else {
    return 0;
  }
}

// NOTE
// When we pop an item out of the array, our array is no longer
// responsible for freeing up the memory. We need to call
// DArray_free() on any element we pop from the array.
void * DArray_pop(DArray * array) {
  check(array->end - 1 >= 0, "Cannot pop from empty array.");

  void * el = DArray_remove(array, array->end - 1);

  array->end--;

  if (DArray_end(array) > (int)array->expand_rate &&
      DArray_end(array) % array->expand_rate) { DArray_contract(array); }

  return el;

error:
  return NULL;
}

void DArray_clear(DArray * array) {
  if (array->element_size > 0) {
    for (int i = 0; i < DArray_max(array); i++) {
      if (DArray_get(array, i) != NULL) {
        DArray_free(DArray_get(array, i));
      }
    }
  }
}

void DArray_destroy(DArray * array) {
  if (array) {
    if (array->contents) {
      DArray_free(array->contents);
    }
    DArray_free(array);
  }
}

void DArray_cleanup(DArray * array) {
  DArray_clear(array);
  DArray_destroy(array);
}

void DArray_qsort(DArray * array, int lo, int hi) {
  if (DArray_count(array) < 2) { return; }

  if (lo < hi) {
    int pivotIndex = DArray_partition(array, lo, hi);

    DArray_qsort(array, lo, pivotIndex);
    DArray_qsort(array, pivotIndex+1, hi);
  }
}

int DArray_partition(DArray * array, int lo, int hi) {
  int pivotIndex = (int)floor(lo + (hi - lo) / 2);
  int * pivotValue = DArray_get(array, pivotIndex);

  int i = lo - 1;
  int j = hi + 1;

  while (1) {
    do { i++; } while(*(int *)DArray_get(array, i) < *pivotValue);
    do { j--; } while(*(int *)DArray_get(array, j) > *pivotValue);

    if (i < j) {
      void * temp = DArray_get(array, j);
      DArray_set(array, j, DArray_get(array, i));
      DArray_set(array, i, temp);
    }
    else {
      return j;
    }
  }
}
