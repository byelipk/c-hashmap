#include "minunit.h"
#include <hashmap/hashmap.h>
#include <assert.h>
#include <lib/bstrlib.h>

Hashmap * map = NULL;
static int traverse_called = 0;
struct tagbstring key1 = bsStatic("test data 1");
struct tagbstring key2 = bsStatic("test data 2");
struct tagbstring key3 = bsStatic("test data 3");
struct tagbstring value1 = bsStatic("THE VALUE 1");
struct tagbstring value2 = bsStatic("THE VALUE 2");
struct tagbstring value3 = bsStatic("THE VALUE 3");

static int traverse_good_cb(HashmapNode * node)
{
    debug("KEY: %s", bdata((bstring)node->key));
    traverse_called++;
    return 0;
}

static int traverse_fail_cb(HashmapNode * node)
{
    debug("KEY: %s", bdata((bstring)node->key));
    traverse_called++;

    if (traverse_called == 2) {
        return 1;
    }
    else {
        return 0;
    }
}

char * test_create()
{
    map = Hashmap_create(NULL, NULL);
    mu_assert(map != NULL, "Failed to create map.");

    return NULL;
}

char * test_destroy()
{
    Hashmap_destroy(map);

    return NULL;
}

char * test_get_set()
{
    int rc = 0;
    bstring result = NULL;

    rc = Hashmap_set(map, &key1, &value1);
    mu_assert(rc == 0, "Failed to set &key1.");
    result = Hashmap_get(map, &key1);
    mu_assert(result == &value1, "Wrong value for key1.");
    bdestroy(result);

    rc = Hashmap_set(map, &key2, &value2);
    mu_assert(rc == 0, "Failed to set &key2.");
    result = Hashmap_get(map, &key2);
    mu_assert(result == &value2, "Wrong value for key2.");
    bdestroy(result);

    rc = Hashmap_set(map, &key3, &value3);
    mu_assert(rc == 0, "Failed to set &key3.");
    result = Hashmap_get(map, &key3);
    mu_assert(result == &value3, "Wrong value for key3.");
    bdestroy(result);

    return NULL;
}

char * test_traverse()
{
    int rc = 0;

    rc = Hashmap_traverse(map, traverse_good_cb);
    mu_assert(rc == 0, "Failed to traverse.");
    mu_assert(traverse_called == 3, "Wrong count after traverse.");

    traverse_called = 0;

    rc = Hashmap_traverse(map, traverse_fail_cb);
    mu_assert(rc == 1, "Failed to traverse.");
    mu_assert(traverse_called == 2, "Wrong count after traverse.");

    return NULL;
}

char * test_delete()
{
    bstring deleted = NULL;
    bstring result = NULL;

    deleted = (bstring) Hashmap_delete(map, &key1);
    mu_assert(deleted != NULL, "Got NULL on delete.");
    mu_assert(deleted == &value1, "Should get key1");
    result = Hashmap_get(map, &key1);
    mu_assert(result == NULL, "Should delete.");
    bdestroy(deleted);
    bdestroy(result);

    deleted = (bstring) Hashmap_delete(map, &key2);
    mu_assert(deleted != NULL, "Got NULL on delete.");
    mu_assert(deleted == &value2, "Should get key2");
    result = Hashmap_get(map, &key2);
    mu_assert(result == NULL, "Should delete.");
    bdestroy(deleted);
    bdestroy(result);

    deleted = (bstring) Hashmap_delete(map, &key3);
    mu_assert(deleted != NULL, "Got NULL on delete.");
    mu_assert(deleted == &value3, "Should get key3");
    result = Hashmap_get(map, &key3);
    mu_assert(result == NULL, "Should delete.");
    bdestroy(deleted);
    bdestroy(result);

    return NULL;
}

char * all_tests()
{
    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_get_set);
    mu_run_test(test_traverse);
    mu_run_test(test_delete);
    mu_run_test(test_destroy);

    return NULL;
}

RUN_TESTS(all_tests);
