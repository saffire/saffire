#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "hashtable/hashtable.h"
#include "dll/dll.h"

int main(int argc, char *argv[]) {

    CU_initialize_registry();

    test_hashtable_init();
    test_dll_init();

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    unsigned int nr_of_fails = CU_get_number_of_tests_failed();
    nr_of_fails += CU_get_number_of_failures();

    CU_cleanup_registry();

    if(nr_of_fails == 0) {
        printf("\n\x1b[1;37;42m\x1b[2K%s\n\x1b[0m\x1b[37;41m\x1b[0m\x1b[2K\n", "Success!");
    } else {
        printf("\n\x1b[1;37;41m\x1b[2K%s\n\x1b[0m\x1b[37;41m\x1b[2K\x1b[0m\x1b[2K\n", "FAILURES!");
    }

    return nr_of_fails > 0 ? 1 : 0;
}
