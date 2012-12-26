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

void test_dll_dll_insert_before_inserts_item_before() {
    t_dll *dll = dll_init();

    dll_append(dll, "test1");
    dll_append(dll, "test3");

    t_dll_element *element = DLL_TAIL(dll);

    dll_insert_before(dll, element, "test2");

    CU_ASSERT_STRING_EQUAL(DLL_NEXT(DLL_HEAD(dll))->data, "test2");
}

void test_dll_dll_insert_after_inserts_item_after() {
    t_dll *dll = dll_init();

    dll_append(dll, "test1");
    dll_append(dll, "test3");

    t_dll_element *element = DLL_HEAD(dll);

    dll_insert_after(dll, element, "test2");

    CU_ASSERT_STRING_EQUAL(DLL_NEXT(DLL_HEAD(dll))->data, "test2");
}

void test_dll_init() {
     CU_pSuite suite = CU_add_suite("dll", NULL, NULL);

     CU_add_test(suite, "dll_init creates object", test_dll_dll_init);
     CU_add_test(suite, "dll_append adds item", test_dll_dll_append_adds_item);
     CU_add_test(suite, "dll_prepend prepends item", test_dll_dll_prepend_prepends_item);
     CU_add_test(suite, "dll_insert_before inserts item before", test_dll_dll_insert_before_inserts_item_before);
     CU_add_test(suite, "dll_insert_after inserts item after", test_dll_dll_insert_after_inserts_item_after);

}