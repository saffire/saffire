#include <CUnit/CUnit.h>
#include <stdio.h>
#include "hashtable.h"
#include "../../src/include/general/hashtable.h"

void test_hashtable_replace_does_not_affect_original_after_shallow_copy() {
    t_hash_table *original = ht_create();
    ht_add_str(original, "key", "original_value");

    t_hash_table *copy = ht_copy(original, 1);

    char *copy_before = (char *) ht_find_str(copy, "key");
    CU_ASSERT(strcmp(copy_before, "original_value") == 0);

    ht_replace_str(copy, "key", "replaced_value");
    char *original_after = (char *) ht_find_str(original, "key");
    CU_ASSERT(strcmp(original_after, "original_value") == 0);

    char *copy_after = (char *) ht_find_str(copy, "key");
    CU_ASSERT(strcmp(copy_after, "replaced_value") == 0);
}


void test_hashtable_init() {
    CU_pSuite suite = CU_add_suite("hashtable", NULL, NULL);
    CU_add_test(suite, "hashtable_copy", test_hashtable_replace_does_not_affect_original_after_shallow_copy);
}

