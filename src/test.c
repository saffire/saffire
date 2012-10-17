/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include "object/object.h"
#include "object/numerical.h"
#include "object/string.h"
#include "object/regex.h"
#include "object/boolean.h"

#define MAX_OBJECTS     100

t_object *obj[MAX_OBJECTS];


/*
 * Initialize objects placeholders ("variables") to NULL
 */
void test_init(void) {
    for (int i=0; i!=MAX_OBJECTS; i++) obj[i] = NULL;
}


/*
 * Free allocated objects
 */
void test_fini(void) {
    for (int i=0; i!=MAX_OBJECTS; i++) {
        if (! obj[i]) continue;
        object_free(obj[i]);
        obj[i] = NULL;
    }
}


/**
 *
 */
void init(void) {
    printf("Object init\n");
    object_init();
    setlocale(LC_ALL,"");

    test_init();
}


/**
 *
 */
void fini(void) {
    test_fini();
    object_fini();
}


/**
 *
 */
void test_arguments(void) {
    test_init();

    printf("=================\n");
    obj[1] = object_new(Object_Regex, L"(o+)", 0);
    obj[2] = object_new(Object_String, L"appooooopoopoo");
    obj[3] = object_call(obj[1], "match", 1, obj[2]);

    printf("=================\n");
    obj[4] = object_new(Object_Regex, L"(o+)", 0);
    obj[7] = object_new(Object_String, L"foofoobarbaz");
    obj[5] = object_call(obj[4], "match", 1, obj[7]);
    obj[6] = object_call(obj[5], "print", 0);

    test_fini();
}


/**
 *
 */
void test_regex(void) {
    test_init();

    obj[0] = object_new(Object_String, L"foofoobarbaz");

    printf("=================\n");
    obj[1] = object_new(Object_Regex, L"(ba+)", 0);
    obj[2] = object_call(obj[1], "match", 1, obj[0]);
    obj[3] = object_call(obj[2], "print", 0);

    printf("=================\n");
    obj[4] = object_new(Object_Regex, L"(BA+)", 0);
    obj[5] = object_call(obj[4], "match", 1, obj[0]);
    obj[6] = object_call(obj[5], "print", 0);

    printf("=================\n");
    obj[7] = object_new(Object_Regex, L"(BA+)", PCRE_CASELESS);
    obj[8] = object_call(obj[7], "match", 1, obj[0]);
    obj[9] = object_call(obj[8], "print", 0);

    test_fini();
}


/**
 *
 */
void test_cast(void) {
    test_init();

    printf("=================\n");
    obj[0] = object_new(Object_Numerical, 123);
    obj[24] = object_call(obj[0], "print", 0);
    obj[1] = object_call(obj[0], "boolean", 0);
    obj[2] = object_call(obj[1], "string", 0);
    obj[25] = object_call(obj[2], "print", 0);

    obj[3] = object_new(Object_Numerical, 0);
    obj[26] = object_call(obj[3], "print", 0);
    obj[4] = object_call(obj[3], "boolean", 0);
    obj[5] = object_call(obj[4], "string", 0);
    obj[27] = object_call(obj[5], "print", 0);

    obj[6] = object_new(Object_Numerical, 1234);
    obj[28] = object_call(obj[6], "print", 0);
    obj[7] = object_call(obj[6], "string", 0);
    obj[29] = object_call(obj[7], "print", 0);

    printf("=================\n");
    obj[8] = object_new(Object_String, (void *)L"");
    obj[9] = object_call(obj[8], "print", 0);
    obj[10] = object_call(obj[8], "boolean", 0);
    obj[11] = object_call(obj[10], "string", 0);
    obj[30] = object_call(obj[11], "print", 0);

    obj[12] = object_new(Object_String, (void *)L"false");
    obj[13] = object_call(obj[12], "print", 0);
    obj[14] = object_call(obj[12], "boolean", 0);
    obj[15] = object_call(obj[14], "string", 0);
    obj[31] = object_call(obj[15], "print", 0);

    obj[16] = object_new(Object_String, (void *)L"true");
    obj[17] = object_call(obj[16], "print", 0);
    obj[18] = object_call(obj[16], "boolean", 0);
    obj[19] = object_call(obj[18], "string", 0);
    obj[32] = object_call(obj[19], "print", 0);

    obj[20] = object_new(Object_String, (void *)L"anythingelse");
    obj[21] = object_call(obj[20], "print", 0);
    obj[22] = object_call(obj[20], "boolean", 0);
    obj[23] = object_call(obj[22], "string", 0);
    obj[33] = object_call(obj[23], "print", 0);

    test_fini();
}


/**
 *
 */
void test_parent_methods(void) {
    test_init();

    printf("=================\n");
    // Simple numerical object called 12345
    obj[1] = object_new(Object_Numerical, 12345);
    obj[2] = object_call(obj[1], "ctor", 0);
    obj[3] = object_call(obj[1], "print", 0);

    // Return a memory prints (not functioning yet)
    obj[4] = object_call(obj[1], "memory", 0);
    obj[5] = object_call(obj[4], "print", 0);

    // Print parents
    obj[6] = object_call(obj[1], "parents", 0);
    obj[7] = object_call(obj[6], "print", 0);

    test_fini();
}


/**
 *
 */
void test_immutable(void) {
    test_init();

    printf("=================\n");

    obj[1] = object_new(Object_Numerical, 12345);

    obj[2] = object_call(obj[1], "immutable?", 0);
    if (obj[2] == Object_True) {
        printf ("Object is immutable\n");
    } else {
        printf ("Object is NOT immutable\n");
    }
    obj[3] = object_call(obj[1], "immutable", 0);
    obj[4] = object_call(obj[1], "immutable?", 0);
    if (obj[4] == Object_True) {
        printf ("Object is immutable\n");
    } else {
        printf ("Object is NOT immutable\n");
    }

    test_fini();
}


/**
 *
 */
void test_string(void) {
    test_init();

    printf("=================\n");

    // Create a string
    obj[1] = object_new(Object_String, (void *)L"Björk Guðmundsdóttir");
    obj[2] = object_call(obj[1], "ctor", 0);
    obj[3] = object_call(obj[1], "print", 0);

    obj[4] = object_call(obj[1], "length", 0);
    printf("Length of the string is: ");
    obj[5] = object_call(obj[4], "print", 0);
    printf("\n");

    obj[6] = object_call(obj[1], "byte_length", 0);
    printf("Byte Length of the string is: ");
    obj[7] = object_call(obj[6], "print", 0);
    printf("\n");

    // Uppercase object
    obj[8] = object_call(obj[1], "upper", 0);
    printf("> obj2::print\n");
    obj[9] = object_call(obj[8], "print", 0);

    // call a method that does not exist
    obj[10] = object_call(obj[8], "sirdoesnotexist", 0);

    // Create a reversed object from the uppercased one
    obj[11] = object_call(obj[8], "reverse", 0);

    // Print reversed uppercase
    obj[12] = object_call(obj[11], "print", 0);

    // Print "normal" string
    obj[13] = object_call(obj[1], "print", 0);

    test_fini();
}


/**
 *
 */
void test_numerical(void) {
    test_init();

    printf("=================\n");

    obj[1] = object_new(Object_Numerical, 123);
    obj[2] = object_call(obj[1], "print", 0);

    // negate the numerical value
    obj[3] = object_call(obj[1], "neg", 0);
    obj[4] = object_call(obj[3], "print", 0);

    test_fini();
}


/**
 *
 */
void test_operators(void) {
    test_init();

    printf("=================\n");

    obj[1] = object_new(Object_Numerical, 10);
    obj[2] = object_operator(obj[1], OPERATOR_ADD, 1, 0);

    obj[3] = object_operator(obj[1], OPERATOR_ADD, 0, 0);
    obj[4] = object_operator(obj[3], OPERATOR_ADD, 1, 0);

    printf("OBJ1: %08X\n", obj[1]);
    printf("OBJ2: %08X\n", obj[2]);
    printf("OBJ3: %08X\n", obj[3]);
    printf("OBJ4: %08X\n", obj[4]);

    obj[5] = object_call(obj[1], "print", 0);
    obj[6] = object_call(obj[2], "print", 0);
    obj[7] = object_call(obj[3], "print", 0);
    obj[8] = object_call(obj[4], "print", 0);

    test_fini();
}


/**
 *
 */
int main(int argc, char *argv[]) {
    init();

    test_arguments();
    test_regex();
    test_cast();
    test_parent_methods();
    test_immutable();
    test_string();
    test_numerical();
    test_operators();

    fini();
}