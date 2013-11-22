#include <CUnit/CUnit.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "ini.h"
#include "../../src/include/general/ini.h"

char tmpfilename[] = "/tmp/cunit_ini_XXXXXX";

static const char *ini_contents[] = {
    "b = test",
    "",
    "[foo]",
    "# Global log information",
    "bar=1",
    "baz=2",
    "",
    "[something]",
    "a.boo=/usr/bin/gpg",
    NULL,
};

t_ini *ini;


/**
 *
 */
static int create_ini(void) {
    int fd = mkstemp(tmpfilename);
    if (fd == -1) return 0;

    int i = 0;
    while (ini_contents[i]) {
        dprintf(fd, "%s\n", ini_contents[i]);
        i++;
    }
    close(fd);

    char *file = basename(tmpfilename);
    char *path = dirname(tmpfilename);

    ini = ini_read(path, file);
    return 1;
}


/**
 *
 */
static int unlink_ini(void) {
    unlink(tmpfilename);

    // Since this is changed by mkstemp, we must make sure it will end on
    strcpy(tmpfilename, "/tmp/cunit_ini_XXXXXX");

    ini_free(ini);
    return 0;
}


/**
 *
 */
void test_ini_simple_reading() {
    if (!create_ini()) CU_FAIL_FATAL("cannot create ini");

    CU_ASSERT_STRING_EQUAL(ini_find(ini, "global.b"), "test");
    CU_ASSERT_STRING_EQUAL(ini_find(ini, "foo.bar"), "1");
    CU_ASSERT_STRING_EQUAL(ini_find(ini, "something.a.boo"), "/usr/bin/gpg");

    CU_ASSERT_PTR_NULL(ini_find(ini, "not.existing"));
    CU_ASSERT_PTR_NULL(ini_find(ini, "global.bar"));

    unlink_ini();
}


/**
 *
 */
void test_ini_simple_matching() {
    if (!create_ini()) CU_FAIL_FATAL("cannot create ini");

    t_hash_table *matches;

    matches = ini_match(ini, "foo.*");
    CU_ASSERT_EQUAL(matches->element_count, 2);
    ht_destroy(matches);

    matches = ini_match(ini, "*bo*");
    CU_ASSERT_EQUAL(matches->element_count, 1);
    ht_destroy(matches);

    matches = ini_match(ini, "*");
    CU_ASSERT_EQUAL(matches->element_count, 4);
    ht_destroy(matches);

    matches = ini_match(ini, "*nothingfound*");
    CU_ASSERT_EQUAL(matches->element_count, 0);
    ht_destroy(matches);

    unlink_ini();
}


/**
 *
 */
void test_ini_simple_add_remove() {
    if (!create_ini()) CU_FAIL_FATAL("cannot create ini");

    t_hash_table *matches;

    CU_ASSERT_STRING_EQUAL(ini_find(ini, "foo.bar"), "1");
    CU_ASSERT_EQUAL(ini_remove(ini, "foo.bar"), 1);
    CU_ASSERT_PTR_NULL(ini_find(ini, "foo.bar"));

    // Cannot remove, does not exist
    CU_ASSERT_EQUAL(ini_remove(ini, "fooooo.bar.nothere"), 0);

    ini_add(ini, "foo.test", "bazbaz");
    CU_ASSERT_PTR_NOT_NULL(ini_find(ini, "foo.test"));
    CU_ASSERT_STRING_EQUAL(ini_find(ini, "foo.test"), "bazbaz");
    matches = ini_match(ini, "foo.*");
    CU_ASSERT_EQUAL(matches->element_count, 2);
    ht_destroy(matches);

    ini_add(ini, "newsection.test", "bazbaz");
    CU_ASSERT_PTR_NOT_NULL(ini_find(ini, "newsection.test"));
    CU_ASSERT_STRING_EQUAL(ini_find(ini, "newsection.test"), "bazbaz");
    matches = ini_match(ini, "*.test");
    CU_ASSERT_EQUAL(matches->element_count, 2);
    ht_destroy(matches);


    CU_ASSERT_EQUAL(ini_remove(ini, "newsection.test"), 1);
    matches = ini_match(ini, "*.test");
    CU_ASSERT_EQUAL(matches->element_count, 1);
    ht_destroy(matches);

    unlink_ini();
}


void test_ini_save() {
    if (!create_ini()) CU_FAIL_FATAL("cannot create ini");
    unlink_ini();
}


void test_ini_init() {
    CU_pSuite suite = CU_add_suite("ini", NULL, NULL);
    CU_add_test(suite, "ini_read", test_ini_simple_reading);
    CU_add_test(suite, "ini_match", test_ini_simple_matching);
    CU_add_test(suite, "ini_add_remove", test_ini_simple_add_remove);

    CU_add_test(suite, "ini_saving", test_ini_save);
}

