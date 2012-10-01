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

    #define VERSION_MAJOR(v)    ((v >> 12) & 0x0004)
    #define VERSION_MINOR(v)    ((v >>  8) & 0x0004)
    #define VERSION_BUILD(v)    ((v >>  0) & 0x0008)
    #define VERSION(maj, min, build) (maj << 12 | min << 8 | build)

    #define MAGIC_HEADER 0x53464243

    typedef struct _bytecode_header {
        uint64_t   magic;              // magic number 0x53464243 (SFBC)
        uint64_t   timestamp;          // Modified timestamp for source file
        uint64_t   version;            // Version

//        uint16_t   index_count;        // Number of indexes
//        uint32_t   index_offset;       // Start of the first index

        uint16_t   constant_count;     // Number of constants in this file
        uint32_t   constant_offset;    // Start of the first constant

        uint16_t   class_count;        // Number of classes in this file
        uint32_t   class_offset;       // Start of the first classes
    } t_bytecode_header;

//    typedef struct _bytecode_index {
//        uint16_t  length;                 // Length of the index
//        uint32_t  offset;                 // Actual offset of the index
//    } t_bytecode_index;

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
    } t_bytecode_class;

    #define CONST_NULL          0
    #define CONST_STRING        1
    #define CONST_NUMERICAL     2
    #define CONST_BOOLEAN       3
    #define CONST_REGEX         4

    typedef struct _bytecode_constant {
        uint8_t    type;            // Type of the constant
        uint16_t   length;          // Length of data
        void       *data;
    } t_bytecode_constant;

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


    char *bytecode_generate(t_ast_element *p, char *source_file, int *len);

#endif