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
#ifndef __SAFFIRE_INTERPRETER_H__
#define __SAFFIRE_INTERPRETER_H__

    #include "compiler/ast.h"
    #include "object/object.h"
    #include "general/dll.h"


    // Count number of operands.
    #define OP_CNT(p) p->opr.nops

    typedef enum { snodeTypeNull, snodeTypeObject, snodeTypeIdentifier, snodeTypeString, snodeTypeDll } snodeTypeEnum;

    struct _snode;

    typedef struct _t_identifier {
        t_object *obj;                  // Actual object
        t_hash_table_bucket *id;        // Pointer to hash-table element which holds the object
    } t_identifier;

    // Snode structure
    typedef struct _snode {
        snodeTypeEnum type;           // Type of the snode
        union {
            void *data;         // Generic data (@TODO: Used?)
            t_object *obj;      // Single object
            t_identifier id;    // An identifier
            char *str;          // Single string
            t_dll *dll;         // DLL
        } data;
    } t_snode;


    // Interpretation macros
    #define SI(p)   (_saffire_interpreter(p))
    #define SI0(p)  (_saffire_interpreter(p->opr.ops[0]))
    #define SI1(p)  (_saffire_interpreter(p->opr.ops[1]))
    #define SI2(p)  (_saffire_interpreter(p->opr.ops[2]))
    #define SI3(p)  (_saffire_interpreter(p->opr.ops[3]))


    // Snode identifier macros
    #define IS_OBJECT(snode)            (snode->type == snodeTypeObject)
    #define IS_IDENTIFIER(snode)        (snode->type == snodeTypeIdentifier)
    #define IS_STRING(snode)            (snode->type == snodeTypeString)
    #define HAS_IDENTIFIER_ID(snode)    (snode->type == snodeTypeIdentifier && snode->data.id.id != NULL)
    #define HAS_IDENTIFIER_OBJ(snode)   (snode->type == snodeTypeIdentifier && snode->data.id.obj != NULL)
    #define IS_DLL(snode)               (snode->type == snodeTypeDll)
    #define IS_NULL(snode)              (snode->type == snodeTypeNull)


    // Snode return macros
    #define RETURN_SNODE_STRING(string) { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                    ret->type = snodeTypeString; \
                                    ret->data.str = string; \
                                    dll_remove(lineno_stack, DLL_TAIL(lineno_stack)); \
                                    return ret; }

    #define RETURN_SNODE_NULL() { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                     ret->type = snodeTypeNull; \
                                     ret->data.data = NULL; \
                                     dll_remove(lineno_stack, DLL_TAIL(lineno_stack)); \
                                     return ret; }

    #define RETURN_SNODE_OBJECT(object) { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                     ret->type = snodeTypeObject; \
                                     ret->data.obj = object; \
                                     dll_remove(lineno_stack, DLL_TAIL(lineno_stack)); \
                                     return ret; }

    #define RETURN_SNODE_IDENTIFIER(ident, object) { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                     ret->type = snodeTypeIdentifier; \
                                     ret->data.id.id = ident; \
                                     ret->data.id.obj = object; \
                                     dll_remove(lineno_stack, DLL_TAIL(lineno_stack)); \
                                     return ret; }

    #define RETURN_SNODE_DLL(_dll) { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                     ret->type = snodeTypeDll; \
                                     ret->data.dll = _dll; \
                                     dll_remove(lineno_stack, DLL_TAIL(lineno_stack)); \
                                     return ret; }


    void saffire_warning(char *str, ...);
    void saffire_error(char *str, ...);
    void saffire_interpreter(t_ast_element *p);

#endif
