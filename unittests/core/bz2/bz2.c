#include <CUnit/CUnit.h>
#include <stdlib.h>
#include <saffire/general/bzip2.h>

#define BZ2_MAXBUFLEN 1024

char str[] = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et "
            "dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex "
            "ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
            "fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum.";

void test_comp_decomp(void) {
    char *buf;
    unsigned int buf_len;

    int ret = bzip2_compress(&buf, &buf_len, str, strlen(str));

    CU_ASSERT_EQUAL(ret, 1);
    CU_ASSERT_EQUAL(buf_len, 302);


    char *buf2 = malloc(BZ2_MAXBUFLEN);
    unsigned int buf2_len = BZ2_MAXBUFLEN;

    ret = bzip2_decompress(buf2, &buf2_len, buf, buf_len);

    CU_ASSERT_EQUAL(ret, 1);
    CU_ASSERT_EQUAL(buf2_len, strlen(str));

    free(buf);
    free(buf2);
}

void test_bz2_init() {
     CU_pSuite suite = CU_add_suite("bz2", NULL, NULL);

     CU_add_test(suite, "bz2 comp/decomp works", test_comp_decomp);
}
