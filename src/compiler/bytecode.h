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

    #define PACKED  __attribute__((packed))

    #define MAGIC_HEADER 0x43424653     // big-endian SFBC (saffire bytecode)

    #define BYTECODE_CONST_NULL          0
    #define BYTECODE_CONST_STRING        1
    #define BYTECODE_CONST_NUMERICAL     2
    #define BYTECODE_CONST_BOOLEAN       3
    #define BYTECODE_CONST_REGEX         4

    typedef struct _bytecode_header {
        uint32_t   magic;              // magic number 0x53464243 (SFBC)
        uint32_t   timestamp;          // Modified timestamp for source file
        uint32_t   version;            // Version

        uint32_t   constant_count;     // Number of constants in this file
        uint32_t   constant_offset;    // Start of the first constant

        uint32_t   class_count;        // Number of classes in this file
        uint32_t   class_offset;       // Start of the first classes
    } PACKED t_bytecode_header;

    typedef struct _bytecode_class {
        uint32_t   name_index;         // Index to the name of the class
        uint32_t   class_flags;        // Class type flags
        uint32_t   extend_index;       // Offset in this file to the extend class (or NULL when not extended)
        uint16_t   interface_count;    // Number of interfaces
        uint32_t   interface_offset;   // Start of the first interface (or NULL when no interfaces)
        uint16_t   constant_count;     // Number of constants
        uint32_t   constant_offset;    // Start of the first constant (or NULL when no constants)
        uint16_t   property_count;     // Number of properties
        uint32_t   property_offset;    // Start of the first property (or NULL when no properties)
        uint16_t   method_count;       // Number of properties
        uint32_t   method_offset;      // Start of the first method (or NULL when no methods)
    } PACKED t_bytecode_class;

    typedef struct _bytecode_constant_header {
        uint32_t   length;          // Length of data
        uint8_t    type;            // Type of the constant
    } PACKED t_bytecode_constant_header;

    typedef struct _bytecode_constant {
        t_bytecode_constant_header hdr;
        union {
            char *s;
            long l;
        } data;
    } t_bytecode_constant;

    typedef struct _bytecode {
        long length;                // Lenght of the complete structure (0 is not known yet)
        char *buffer;               // Binary buffer
        t_bytecode_header *header;
        t_dll *constant_dll;
        t_dll *class_dll;
    } t_bytecode;


//    typedef struct _bytecode_property {
//    } t_bytecode_property;
//
//    typedef struct _bytecode_method {
//    } t_bytecode_method;

//    typedef struct _bytecode_line {
//        char *filename;     // Which file in the source does this bytecode represent
//        int line;           // Which line in the source does this bytecode represent
//
//        // @TODO: Add bytecode stuff
//    } t_bytecode_line;


    t_bytecode *bytecode_generate(t_ast_element *p, char *source_file);
    void bytecode_free(t_bytecode *bc);
    char *bytecode_generate_destfile(const char *src);

#endif