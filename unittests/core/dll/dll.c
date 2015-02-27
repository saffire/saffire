#include <CUnit/CUnit.h>
#include "dll.h"
#include "../../include/general/dll.h"


static void test_dll_dll_init() {
    t_dll *dll = dll_init();

    CU_ASSERT_PTR_NOT_NULL(dll);
    CU_ASSERT_EQUAL(dll->size, 0);

    dll_free(dll);
}

static void test_dll_dll_append_adds_item() {
    t_dll *dll = dll_init();

    dll_append(dll, "test");

    CU_ASSERT_STRING_EQUAL(DLL_HEAD(dll)->data, "test");

    dll_free(dll);
}

static void test_dll_dll_prepend_prepends_item() {
    t_dll *dll = dll_init();

    dll_append(dll, "test");
    dll_prepend(dll, "prepend");

    CU_ASSERT_STRING_EQUAL(DLL_HEAD(dll)->data, "prepend");

    dll_free(dll);
}

static void test_dll_dll_insert_before_inserts_item_before() {
    t_dll *dll = dll_init();

    dll_insert_before(dll, NULL, "test3");
    dll_insert_before(dll, NULL, "test1");

    t_dll_element *element = DLL_TAIL(dll);
    dll_insert_before(dll, element, "test2");

    CU_ASSERT_STRING_EQUAL(DLL_HEAD(dll)->data, "test1");
    CU_ASSERT_STRING_EQUAL(DLL_NEXT(DLL_HEAD(dll))->data, "test2");
    CU_ASSERT_STRING_EQUAL(DLL_NEXT(DLL_NEXT(DLL_HEAD(dll)))->data, "test3");

    element = DLL_HEAD(dll);
    dll_insert_before(dll, element, "test0");
    CU_ASSERT_STRING_EQUAL(DLL_HEAD(dll)->data, "test0");

    dll_free(dll);
}

static void test_dll_dll_insert_after_inserts_item_after() {
    t_dll *dll = dll_init();

    dll_append(dll, "test1");
    dll_append(dll, "test3");

    t_dll_element *element = DLL_HEAD(dll);

    dll_insert_after(dll, element, "test2");

    CU_ASSERT_STRING_EQUAL(DLL_HEAD(dll)->data, "test1");
    CU_ASSERT_STRING_EQUAL(DLL_NEXT(DLL_HEAD(dll))->data, "test2");
    CU_ASSERT_STRING_EQUAL(DLL_NEXT(DLL_NEXT(DLL_HEAD(dll)))->data, "test3");

    element = DLL_TAIL(dll);
    dll_insert_after(dll, element, "test4");
    CU_ASSERT_STRING_EQUAL(DLL_TAIL(dll)->data, "test4");

    dll_free(dll);
}

static void test_dll_dll_push() {
    t_dll *dll = dll_init();

    dll_push(dll, "test1");
    CU_ASSERT_STRING_EQUAL(DLL_TAIL(dll)->data, "test1");

    dll_push(dll, "test2");
    CU_ASSERT_STRING_EQUAL(DLL_TAIL(dll)->data, "test2");

    dll_free(dll);
}

//static void test_dll_dll_pop() {
//    char *data;
//    t_dll *dll = dll_init();
//
//    dll_push(dll, "test1");
//    dll_push(dll, "test2");
//
//    data = dll_pop(dll);
//    CU_ASSERT_PTR_NOT_NULL(data);
//    CU_ASSERT_STRING_EQUAL(data, "test2");
//
//    data = dll_pop(dll);
//    CU_ASSERT_PTR_NOT_NULL(data);
//    CU_ASSERT_STRING_EQUAL(data, "test1");
//
//    data = dll_pop(dll);
//    CU_ASSERT_PTR_NULL(data);
//
//    dll_free(dll);
//}

//static void test_dll_dll_top() {
//    char *data;
//    t_dll *dll = dll_init();
//
//    dll_push(dll, "test1");
//    dll_push(dll, "test2");
//
//    data = dll_top(dll);
//    CU_ASSERT_STRING_EQUAL(data, "test2");
//
//    data = dll_top(dll);
//    CU_ASSERT_STRING_EQUAL(data, "test2");
//
//    dll_pop(dll);
//    data = dll_top(dll);
//    CU_ASSERT_STRING_EQUAL(data, "test1");
//
//    dll_free(dll);
//}

static void test_dll_dll_remove() {
    t_dll *dll;
    char *data;

    // Removing first element

    dll = dll_init();

    dll_push(dll, "test1");
    dll_push(dll, "test2");
    dll_push(dll, "test3");

    dll_remove(dll, DLL_HEAD(dll));

    data = dll_pop(dll);
    CU_ASSERT_PTR_NOT_NULL(data);
    CU_ASSERT_STRING_EQUAL(data, "test3");
    data = dll_pop(dll);
    CU_ASSERT_PTR_NOT_NULL(data);
    CU_ASSERT_STRING_EQUAL(data, "test2");

    dll_free(dll);

    // Removing middle element

    dll = dll_init();

    dll_push(dll, "test1");
    dll_push(dll, "test2");
    dll_push(dll, "test3");

    dll_remove(dll, DLL_NEXT(DLL_HEAD(dll)));

    data = dll_pop(dll);
    CU_ASSERT_PTR_NOT_NULL(data);
    CU_ASSERT_STRING_EQUAL(data, "test3");
    data = dll_pop(dll);
    CU_ASSERT_PTR_NOT_NULL(data);
    CU_ASSERT_STRING_EQUAL(data, "test1");

    dll_free(dll);


    // Removing last element

    dll = dll_init();

    dll_push(dll, "test1");
    dll_push(dll, "test2");
    dll_push(dll, "test3");

    dll_remove(dll, DLL_TAIL(dll));

    data = dll_pop(dll);
    CU_ASSERT_PTR_NOT_NULL(data);
    CU_ASSERT_STRING_EQUAL(data, "test2");
    data = dll_pop(dll);
    CU_ASSERT_PTR_NOT_NULL(data);
    CU_ASSERT_STRING_EQUAL(data, "test1");

    dll_free(dll);

}

void test_dll_init() {
     CU_pSuite suite = CU_add_suite("dll", NULL, NULL);

     CU_add_test(suite, "dll_init creates object", test_dll_dll_init);
     CU_add_test(suite, "dll_append adds item", test_dll_dll_append_adds_item);
     CU_add_test(suite, "dll_prepend prepends item", test_dll_dll_prepend_prepends_item);
     CU_add_test(suite, "dll_insert_after inserts item after", test_dll_dll_insert_after_inserts_item_after);
     CU_add_test(suite, "dll_insert_before inserts item before", test_dll_dll_insert_before_inserts_item_before);
     CU_add_test(suite, "dll_remove", test_dll_dll_remove);

     CU_add_test(suite, "dll_push", test_dll_dll_push);
//     CU_add_test(suite, "dll_pop", test_dll_dll_pop);
//     CU_add_test(suite, "dll_top", test_dll_dll_top);

}
