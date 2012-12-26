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

void test_dll_dll_prepend_prepends_item() {
    t_dll *dll = dll_init();

    dll_append(dll, "test");
    dll_prepend(dll, "prepend");

    CU_ASSERT_STRING_EQUAL(DLL_HEAD(dll)->data, "prepend");
}

void test_dll_init() {
     CU_pSuite suite = CU_add_suite("dll", NULL, NULL);

     CU_add_test(suite, "dll_init creates object", test_dll_dll_init);
     CU_add_test(suite, "dll_append adds item", test_dll_dll_append_adds_item);
     CU_add_test(suite, "dll_prepend prepends item", test_dll_dll_prepend_prepends_item);

}