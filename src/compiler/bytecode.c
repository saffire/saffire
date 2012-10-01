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
#include "compiler/bytecode.h"
#include "compiler/ast.h"
#include "general/dll.h"
#include "general/smm.h"


/**
 *
 */
static void _bytecode_parse_constants(t_ast_element *p, t_dll *constant_dll, int *len) {
    t_bytecode_constant *constant;
    if (!p) return;

    switch (p->type) {
        case typeString :
            constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
            constant->type = CONST_STRING;
            constant->length = 3 + strlen(p->string.value);
            constant->data = strdup(p->string.value);
            dll_append(constant_dll, constant);
            break;
        case typeNumerical :
            constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
            constant->type = CONST_NUMERICAL;
            constant->length = 3 + sizeof(long);
            constant->data = (long *)malloc(sizeof(long));
            constant->data = &p->numerical.value;
            dll_append(constant_dll, constant);
            break;
        case typeIdentifier :
            constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
            constant->type = CONST_STRING;
            constant->length = 3 + strlen(p->identifier.name);
            constant->data = strdup(p->identifier.name);
            dll_append(constant_dll, constant);
            break;
        case typeOpr :
            // Plot all the operands
            for (int i=0; i!=p->opr.nops; i++) {
                _bytecode_parse_constants(p->opr.ops[i], constant_dll, len);
            }
            break;
        case typeNull :
            constant = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
            constant->type = CONST_NULL;
            constant->length = 3;
            constant->data = 0;
            dll_append(constant_dll, constant);
            break;
        case typeInterface :
            _bytecode_parse_constants(p->interface.implements, constant_dll, len);
            _bytecode_parse_constants(p->interface.body, constant_dll, len);
            break;
        case typeClass :
            _bytecode_parse_constants(p->class.extends, constant_dll, len);
            _bytecode_parse_constants(p->class.implements, constant_dll, len);
            _bytecode_parse_constants(p->class.body, constant_dll, len);
            break;
        case typeMethod:
            _bytecode_parse_constants(p->method.arguments, constant_dll, len);
            _bytecode_parse_constants(p->method.body, constant_dll, len);
            break;
        default :
            printf("Unknown type!");
            exit(1);
            break;
    }
}


/**
 *
 */
char *bytecode_generate(t_ast_element *p, char *source_file, int *len) {
    t_dll_element *e;
    len = 0;

    // Create initial header
    t_bytecode_header *hdr = (t_bytecode_header *)smm_malloc(sizeof(t_bytecode_header));
    memset(hdr, 0, sizeof(t_bytecode_header));
    len += sizeof(t_bytecode_header);

    // Fetch mtime from source file
    struct stat sb;
    long mtime = 0;
    if (! stat(source_file, &sb)) {
        mtime = sb.st_mtime;
    }

    hdr->magic = MAGIC_HEADER;
    hdr->timestamp = mtime;


    // Create and parse the constant DLL
    t_dll *constant_dll = dll_init();
    _bytecode_parse_constants(p, constant_dll, len);


    // Create data buffer and fill
    char *buf = (char *)smm_malloc(len);
    int bufpos = 0;
    memcpy(buf+bufpos, hdr, sizeof(t_bytecode_header));
    bufpos += sizeof(t_bytecode_header);

    // Add all entries to the index
    e = DLL_HEAD(constant_dll);
    for (int i=0; i!=constant_dll->size; i++) {
        t_bytecode_constant *constant = e->data;
        memcpy(buf+bufpos, constant, constant->length);
        bufpos += constant->length;
        e = e->next;
    }


    // Free all
    dll_free(constant_dll);
    smm_free(hdr);

    return buf;
}