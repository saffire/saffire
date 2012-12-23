#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "hashtable/hashtable.h"

int main(int argc, char *argv[]) {

    CU_initialize_registry();

    test_hashtable_init();

    CU_basic_run_tests();

    unsigned int nr_of_fails = CU_get_number_of_tests_failed();
    nr_of_fails += CU_get_number_of_failures();

    CU_cleanup_registry();

    return nr_of_fails > 0 ? 1 : 0;
}