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
#ifndef __BYTECODE_H__
#define __BYTECODE_H__

    #include <stdint.h>
    #include "compiler/ast.h"
    #include "general/dll.h"
    #include "objects/object.h"

    #define PACKED  __attribute__((packed))

    #define MAGIC_HEADER            0x43424653     // big-endian SFBC (saffire bytecode)

    #define BYTECODE_CONST_STRING        0
    #define BYTECODE_CONST_NUMERICAL     1
    #define BYTECODE_CONST_CODE          2
    #define BYTECODE_CONST_OBJECT        3


    typedef struct _bytecode_binary_header {
        uint32_t   magic;                       // Magic number 0x53464243 (SFBC)
        uint32_t   timestamp;                   // Modified timestamp for source file
        uint32_t   bytecode_len;                // Length of the bytecode
        uint32_t   bytecode_offset;             // Offset of the bytecode
        uint32_t   signature_len;               // Length of the GPG check (NULL of none)
        uint32_t   signature_offset;            // Offset of the GPG signature (NULL if none)
    } PACKED t_bytecode_binary_header;

    struct _bytecode;
    typedef struct _bytecode_constant_header {
        char type;          // Type of the constant
        int  len;           // Length of data

        union {
            char *s;                    // String
            long l;                     // Long
            struct _bytecode *code;     // Code block
            void *ptr;                  // Generic pointer
            t_object *obj;              // Object
        } data;
    } t_bytecode_constant;


    typedef struct _bytecode_variable_header {
        int  len;           // Length of data
        char *s;            // Name of the variable
    } t_bytecode_variable;


    typedef struct _bytecode {
        int stack_size;         // Maximum stack size for this bytecode

        int code_len;           // Length of the code
        char *code;             // Actual binary code

        int constants_len;                  // Number of constants
        t_bytecode_constant **constants;    // Pointer to constant array

        int variables_len;                  // Number of variables
        t_bytecode_variable **variables;    // Pointer to variables array
    } t_bytecode;


    t_bytecode *generate_dummy_bytecode(void);
    t_bytecode *bytecode_generate(t_ast_element *p, char *source_file);
    void bytecode_free(t_bytecode *bc);
    char *bytecode_generate_destfile(const char *src);

#endif