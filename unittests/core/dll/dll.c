#include <CUnit/CUnit.h>
#include "dll.h"
#include "../../src/include/general/dll.h"


void test_dll_dll_init() {
    t_dll *dll = dll_init();

    CU_ASSERT_PTR_NOT_NULL(dll);
    CU_ASSERT_EQUAL(dll->size, 0);
}

void test_dll_dll_append_adds_item() {
    t_dll *dll = dll_init();

    dll_append(dll, "test");

    CU_ASSERT_STRING_EQUAL(DLL_HEAD(dll)->data, "test");
}

void test_dll_init() {
     CU_pSuite suite = CU_add_suite("dll", NULL, NULL);

     CU_add_test(suite, "dll_init", test_dll_dll_init);
     CU_add_test(suite, "dll_append_adds_item", test_dll_dll_append_adds_item);
}