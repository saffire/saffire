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
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "compiler/bytecode.h"
#include "compiler/ast.h"
#include "general/dll.h"
#include "general/smm.h"
#include "version.h"
#include "vm/vm_opcodes.h"

//
///**
// * Check if a constant already is present inside the bytecode structure
// */
//static int _bytecode_constant_exists(t_bytecode *bc, int type, void *value) {
//    t_dll_element *e = DLL_HEAD(bc->constants_dll);
//    long l;
//
//    while (e) {
//        t_bytecode_constant *constant = e->data;
//        if (constant->hdr.type == type) {
//            switch (constant->hdr.type) {
//                case BYTECODE_CONST_NULL :
//                    return 1;
//                    break;
//                case BYTECODE_CONST_STRING :
//                    if (strcmp(constant->data.s, value) == 0) return 1;
//                    break;
//                case BYTECODE_CONST_NUMERICAL :
//                    printf("Checking %d against %d\n", constant->data.l, (long)value);
//                    if (constant->data.l == (long)value) return 1;
//                    break;
//                case BYTECODE_CONST_BOOLEAN :
//                    if (constant->data.l == (long)value) return 1;
//                    break;
//                case BYTECODE_CONST_REGEX :
//                    break;
//            }
//        }
//        e = DLL_NEXT(e);
//    }
//
//    return 0;
//}
//
///**
// * Parse the AST and add all constants
// */
//static void _bytecode_parse_constants(t_ast_element *p, t_bytecode *bc) {
//    t_bytecode_constant *constant = NULL;
//    if (!p) return;
//
//    switch (p->type) {
////        case typeAstString :
////            printf("STRING: '%s'\n", p->string.value);
////            if (! _bytecode_constant_exists(bc, BYTECODE_CONST_STRING, p->string.value)) {
////                constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
////                constant->hdr.type = BYTECODE_CONST_STRING;
////                constant->hdr.length = strlen(p->string.value);
////                constant->data.s = smm_strdup(p->string.value);
////                dll_append(bc->constant_dll, constant);
////            }
////            break;
////        case typeAstNumerical :
////            if (! _bytecode_constant_exists(bc, BYTECODE_CONST_NUMERICAL, (void *)p->numerical.value)) {
////                constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
////                constant->hdr.type = BYTECODE_CONST_NUMERICAL;
////                constant->hdr.length = sizeof(long);
////                constant->data.l = p->numerical.value;
////                dll_append(bc->constant_dll, constant);
////            }
////            break;
////        case typeAstIdentifier :
////            if (p->string.value[0] == '$') break;
////            printf("IDENT: '%s'\n", p->string.value);
////            if (! _bytecode_constant_exists(bc, BYTECODE_CONST_STRING, p->identifier.name)) {
////                constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
////                constant->hdr.type = BYTECODE_CONST_STRING;
////                constant->hdr.length = strlen(p->identifier.name);
////                constant->data.s = smm_strdup(p->identifier.name);
////                dll_append(bc->constant_dll, constant);
////            }
////            break;
////        case typeNull :
////            if (! _bytecode_constant_exists(bc, BYTECODE_CONST_NULL, NULL)) {
////                constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
////                constant->hdr.type = BYTECODE_CONST_NULL;
////                constant->hdr.length = 0;
////                constant->data.l = 0;
////                dll_append(bc->constant_dll, constant);
////            }
////            break;
////        case typeOpr :
////            // Plot all the operands
////            for (int i=0; i!=p->opr.nops; i++) {
////                _bytecode_parse_constants(p->opr.ops[i], bc);
////            }
////            break;
////        case typeInterface :
////            _bytecode_parse_constants(p->interface.implements, bc);
////            _bytecode_parse_constants(p->interface.body, bc);
////            break;
////        case typeClass :
////            _bytecode_parse_constants(p->class.extends, bc);
////            _bytecode_parse_constants(p->class.implements, bc);
////            _bytecode_parse_constants(p->class.body, bc);
////            break;
////        case typeMethod:
////            _bytecode_parse_constants(p->method.arguments, bc);
////            _bytecode_parse_constants(p->method.body, bc);
////            break;
//        default :
//            printf("Unknown type!");
//            exit(1);
//            break;
//    }
//}
//
//
///**
// * Returns the length of byte from a (filled) bytecode structure.
// */
//static long bytecode_calculate_length(t_bytecode *bc) {
//    t_dll_element *e;
//    long len = sizeof(t_bytecode_header);
//
//    // Calculate and add constant lengths
//    if (bc->constant_dll) {
//        e = DLL_HEAD(bc->constant_dll);
//        for (int i=0; i!=bc->constant_dll->size; i++) {
//            t_bytecode_constant *constant = e->data;
//            len += sizeof(t_bytecode_constant_header) + constant->hdr.length;
//            e = DLL_NEXT(e);
//        }
//    }
//
//    // Calculate and add class lengths
//    if (bc->class_dll) {
//        e = DLL_HEAD(bc->class_dll);
//        for (int i=0; i!=bc->class_dll->size; i++) {
//            t_bytecode_constant *constant = e->data;
//            len += sizeof(t_bytecode_constant_header) + constant->hdr.length;
//            e = DLL_NEXT(e);
//        }
//    }
//
//    return len;
//}
//
///**
// *
// */
//static void bytecode_generate_buffer(t_bytecode *bc) {
//    t_dll_element *e;
//    int bufpos = 0;
//
//    // Create our binary bytecode buffer
//    bc->length = bytecode_calculate_length(bc);
//    bc->buffer = (char *)smm_malloc(bc->length);
//
//    // Skip the header. We still need to fill some vars first
//    bufpos = sizeof(t_bytecode_header);
//
//    printf("CDS: %d\n", bc->constant_dll->size);
//    printf("CDO: %08X (%d)\n", bufpos, bufpos);
//
//    // Add constant header vars
//    bc->header->constant_count = bc->constant_dll->size;
//    bc->header->constant_offset = bufpos;
//
//    // Add all constant entries
//    e = DLL_HEAD(bc->constant_dll);
//    for (int i=0; i!=bc->constant_dll->size; i++) {
//        t_bytecode_constant *constant = e->data;
//
//        // Save header
//        memcpy(bc->buffer+bufpos, &constant->hdr, sizeof(t_bytecode_constant_header));
//        bufpos += sizeof(t_bytecode_constant_header);
//
//        // Save additional data
//        if (constant->hdr.length) {
//            if (constant->hdr.type == BYTECODE_CONST_STRING || constant->hdr.type == BYTECODE_CONST_REGEX) {
//                memcpy(bc->buffer+bufpos, constant->data.s, constant->hdr.length);
//            } else {
//                memcpy(bc->buffer+bufpos, &constant->data.l, constant->hdr.length);
//            }
//            bufpos += constant->hdr.length;
//        }
//
//        e = DLL_NEXT(e);
//    }
//
//
//    // Add class header vars
//    bc->header->class_count = 0x0;
//    bc->header->class_offset = bufpos;
//
//    // Add all constant entries
//    e = DLL_HEAD(bc->class_dll);
//    // @TODO: fill this
//
//    // Finally, go back and fill the header.
//    bufpos = 0;
//    memcpy(bc->buffer+bufpos, bc->header, sizeof(t_bytecode_header));
//}
//
//
//
///**
// * Allocate the bytecode structure
// */
//static t_bytecode *bytecode_structure_alloc(void) {
//    t_bytecode *bc = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
//
//    bc->length = 0;
//    bc->header = (t_bytecode_header *)smm_malloc(sizeof(t_bytecode_header));
//    bc->constant_dll = dll_init();
//    bc->class_dll = dll_init();
//
//    memset(bc->header, 0, sizeof(t_bytecode_header));
//
//    return bc;
//}
//
//
///**
// * Release the bytecode structure
// */
//void bytecode_free(t_bytecode *bc) {
//    dll_free(bc->class_dll);
//    dll_free(bc->constant_dll);
//    smm_free(bc->header);
//    smm_free(bc);
//}
//
//
///**
// * Generate a complete binary bytecode buffer from an AST.
// */
//t_bytecode *bytecode_generate(t_ast_element *p, char *source_file) {
//    t_bytecode *bc = bytecode_structure_alloc();
//
//    // Fetch modification time from source file and fill into header
//    struct stat sb;
//    if (! stat(source_file, &sb)) {
//        bc->header->timestamp = sb.st_mtime;
//    }
//    bc->header->magic = MAGIC_HEADER;
//    bc->header->version = saffire_version_binary;
//
//
//    _bytecode_parse_constants(p, bc);
//    //_bytecode_parse_classes(p, bc);
//
//    bytecode_generate_buffer(bc);
//    return bc;
//}
//
//
//char *bytecode_generate_destfile(const char *src) {
//    printf("SRC: '%s'\n", src);
//    char *dst = (char *)smm_malloc(strlen(src)+2);
//    memset(dst, 0, strlen(src)+2);
//    strcpy(dst,src);
//    dst[strlen(dst)] = 'c'; // Add extra c to create *.sfc
//
//    printf("DST: '%s'\n", dst);
//    return dst;
//}




/**
 * Add a new constant to the bytecode structure
 */
static void _new_constant(t_bytecode *bc, int type, int len, void *data) {
    t_bytecode_constant *c = smm_malloc(sizeof(t_bytecode_constant));
    c->type = type;
    c->len = len;
    c->data.ptr = (void *)data;

    bc->constants = smm_realloc(bc->constants, sizeof(t_bytecode_constant *) * (bc->constants_len + 1));
    bc->constants[bc->constants_len] = c;
    bc->constants_len++;
}

/**
 * Add a new variable to the bytecode structure
 */
static void _new_variable(t_bytecode *bc, char *var) {
    t_bytecode_variable *c = smm_malloc(sizeof(t_bytecode_variable));
    c->len = strlen(var);
    c->s = var;

    bc->variables = smm_realloc(bc->variables, sizeof(t_bytecode_variable *) * (bc->variables_len + 1));
    bc->variables[bc->variables_len] = c;
    bc->variables_len++;
}


/**
 * Create dummy bytecode instance
 * @TODO remove me
 */
t_bytecode *generate_dummy_bytecode(void) {
    char dummy_code[] =
                        "\x64\x00\x00\x00\x00"      // LOAD_CONST   0 (0x1234)
                        "\x5a\x00\x00\x00\x00"      // STORE_VAR    0 (a)
                        "\x64\x01\x00\x00\x00"      // LOAD_CONST   1 (0x5678)
                        "\x5a\x01\x00\x00\x00"      // STORE_VAR    1 (b)
                        "\x65\x00\x00\x00\x00"      // LOAD_VAR     0 (a)
                        "\x59"                      // PRINT_VAR
                        "\x65\x01\x00\x00\x00"      // LOAD_VAR     1 (b)
                        "\x59"                      // PRINT_VAR
                        "\x65\x00\x00\x00\x00"      // LOAD_VAR     0 (a)
                        "\x65\x01\x00\x00\x00"      // LOAD_VAR     1 (b)
                        "\x02"                      // ROT_TWO
                        "\x5a\x00\x00\x00\x00"      // STORE_VAR    0 (a)
                        "\x5a\x01\x00\x00\x00"      // STORE_VAR    1 (b)
                        "\x65\x00\x00\x00\x00"      // LOAD_VAR     0 (a)
                        "\x59"                      // PRINT_VAR
                        "\x65\x01\x00\x00\x00"      // LOAD_VAR     1 (b)
                        "\x59"                      // PRINT_VAR
                        "\x00"                      // STOP
                       ;

    t_bytecode *bc = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
    bc->stack_size = 10;
    bc->code_len = sizeof(dummy_code);
    bc->code = smm_malloc(bc->code_len);
    memcpy(bc->code, dummy_code, bc->code_len);

    bc->constants = NULL;
    bc->variables = NULL;

    // constants
    _new_constant(bc, BYTECODE_CONST_NUMERICAL, 4, (void *)0x1234);
    _new_constant(bc, BYTECODE_CONST_NUMERICAL, 4, (void *)0x5678);
    _new_variable(bc, "a");
    _new_variable(bc, "b");
}
