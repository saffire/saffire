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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter/saffire_interpreter.h"
#include "compiler/parser.tab.h"
#include "general/svar.h"
#include "general/hashtable.h"
#include "compiler/ast.h"
#include "general/smm.h"
#include "object/object.h"
#include "object/string.h"
#include "object/numerical.h"
#include "object/null.h"
#include "object/boolean.h"

extern char *get_token_string(int token);

#define SI(p)   (_saffire_interpreter(p))
#define SI0(p)  (_saffire_interpreter(p->opr.ops[0]))
#define SI1(p)  (_saffire_interpreter(p->opr.ops[1]))
#define SI2(p)  (_saffire_interpreter(p->opr.ops[2]))

t_hash_table *vars;


/**
 * Print out an error and exit
 */
static void saffire_error(char *str, ...) {
    va_list args;
    va_start(args, str);
    printf("Error: ");
    vprintf(str, args);
    printf("\n");
    va_end(args);
    exit(1);
}

static void saffire_warning(char *str, ...) {
    va_list args;
    va_start(args, str);
    printf("Warning: ");
    vprintf(str, args);
    printf("\n");
    va_end(args);
}


#define CNT(p) p->opr.nops

#define SNODE_NULL          0
#define SNODE_OBJECT        1
#define SNODE_METHOD        2
#define SNODE_VARIABLE      3

typedef struct _snode {
    char    type;
    void    *data;
} t_snode;


#define IS_OBJECT(snode)  (snode->type == SNODE_OBJECT)
#define IS_METHOD(snode)  (snode->type == SNODE_METHOD)
#define IS_VARIABLE(snode)  (snode->type == SNODE_VARIABLE)

#define RETURN_SNODE_NULL() { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                 ret->type = SNODE_NULL; \
                                 ret->data = NULL; \
                                 return ret; }

#define RETURN_SNODE_OBJECT(obj) { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                 ret->type = SNODE_OBJECT; \
                                 ret->data = obj; \
                                 return ret; }

#define RETURN_SNODE_METHOD(method) { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                 ret->type = SNODE_METHOD; \
                                 ret->data = method; \
                                 return ret; }

#define RETURN_SNODE_VARIABLE(var) { t_snode *ret = (t_snode *)smm_malloc(sizeof(t_snode)); \
                                 ret->type = SNODE_VARIABLE; \
                                 ret->data = var; \
                                 return ret; }



/**
 *
 */
static t_object *si_obj(t_ast_element *p, int idx) {
    if (idx > CNT(p)) {
        saffire_warning("Warning, want index %d, but max is %d\n", idx, CNT(p));
    }

    t_snode *snode = (t_snode *)p->opr.ops[idx];
    if (snode->type != SNODE_OBJECT) {
        saffire_warning("Warning, need an object, but got a %d\n", snode->type);
    }
    return snode->data;
}


/**
 * Returns 1 when node is writable (mutable object), 0 otherwise
 */
static int si_writable(t_snode *node) {
    if (IS_OBJECT(node) && ! object_is_immutable((t_object *)node->data)) return 1;
    return 0;
}



static t_snode *si_cmp(t_ast_element *p, int cmp);
static t_snode *si_opr(t_ast_element *p, int opr);


static t_object *si_get_object(t_snode *node) {
    t_object *obj;

    if (IS_VARIABLE(node)) {
        // Fetch object where this var references to, and add it to the destination var
        t_hash_table_bucket *htb = node->data;
        obj = (t_object *)htb->data;
        if (obj == NULL) {
            saffire_error("This variable is not initialized!");
        }

    } else {
        // Fetch object where this var references to, and add it to the destination var
        obj = (t_object *)node->data;
    }

    return obj;
}

/**
 *
 */
static t_snode *_saffire_interpreter(t_ast_element *p) {
    t_object *obj, *obj1, *obj2, *obj3;
    t_snode *node1, *node2, *node3;
    t_hash_table_bucket *htb;
    int ret, initial_loop;

    if (!p) {
        RETURN_SNODE_OBJECT(Object_Null);
    }

    wchar_t *wchar_tmp;
    switch (p->type) {
        case typeString :
            printf ("new string object: %s\n", p->string.value);

            // Allocate enough room to hold string in wchar and convert
            wchar_tmp = (wchar_t *)malloc(strlen(p->string.value)*sizeof(wchar_t));
            mbstowcs(wchar_tmp, p->string.value, strlen(p->string.value));

            // create string object
            obj = object_new(Object_String, wchar_tmp);

            // Free tmp wide string
            smm_free(wchar_tmp);
            RETURN_SNODE_OBJECT(obj);

        case typeNumerical :
            printf ("numerical: %d\n", p->numerical.value);
            // Create numerical object
            obj = object_new(Object_Numerical, p->numerical.value);
            RETURN_SNODE_OBJECT(obj);

        case typeIdentifier :
            // Do constant vars
            if (strcasecmp(p->string.value, "True") == 0) {
                RETURN_SNODE_OBJECT(Object_True);
            }
            if (strcasecmp(p->string.value, "False") == 0) {
                RETURN_SNODE_OBJECT(Object_False);
            }
            if (strcasecmp(p->string.value, "Null") == 0) {
                RETURN_SNODE_OBJECT(Object_Null);
            }

            htb = ht_find(vars, p->string.value);
            if (htb == NULL) {
                printf("Creating a new entry for %s\n", p->string.value);
                ht_add(vars, p->string.value, NULL);    // set to NULL by default
                htb = ht_find(vars, p->string.value);
            }

            // Return variable, otherwise it's a method (name)
            if (p->string.value[0] == '$') {
                RETURN_SNODE_VARIABLE(htb);
            } else {
                RETURN_SNODE_METHOD(htb);
            }

        case typeOpr :
            printf ("opr.oper: %s(%d)\n", get_token_string(p->opr.oper), p->opr.oper);
            switch (p->opr.oper) {
                case T_PROGRAM :
                    SI0(p);
                    SI1(p);
                    break;
                case T_TOP_STATEMENTS:
                case T_USE_STATEMENTS:
                case T_STATEMENTS :
                    for (int i=0; i!=CNT(p); i++) {
                        _saffire_interpreter(p->opr.ops[i]);
                    }
                    RETURN_SNODE_NULL();
                    break;
                case T_USE :
                    // @TODO: Use statements are not operational
                    saffire_warning("Use statements are not functional");
                    break;

                case T_EXPRESSIONS :
                    // Return first expression result
                    for (int i=0; i!=CNT(p); i++) {
                        node1 = _saffire_interpreter(p->opr.ops[i]);
                        if (i == 0) node2 = node1;
                    }
                    return node2;
                    break;

                case T_ASSIGNMENT :
                    node1 = SI0(p);
                    if (! IS_VARIABLE(node1)) {
                        saffire_error("Left hand side is not writable!");
                    }

                    // @TODO: operator should be =, but we don't check for now.. :/
//                    node2 = SI1(p);

                    // Evaluate
                    node3 = SI2(p);

                    // Already something present, since we are loosing this, decrease it's reference count
                    t_hash_table_bucket *htb = node1->data;
                    if (htb->data != NULL) {
                        obj1 = (t_object *)htb->data;
                        object_dec_ref(obj1);
                    }

                    // Get the object and save it in our variable hashtable
                    obj1 = si_get_object(node3);
                    htb->data = obj1;

                    // increase it's reference
                    object_inc_ref(obj1);
                    RETURN_SNODE_OBJECT(obj1);
                    break;

                case T_WHILE :
                    initial_loop = 1;
                    while (1) {
                        node1 = SI0(p);
                        obj1 = si_get_object(node1);
                        // Check if it's already a boolean. If not, cast this object to boolean
                        if (! OBJECT_IS_BOOLEAN(obj1)) {
                            obj1 = object_call(obj1, "boolean", 0);
                        }
                        if (obj1 == Object_True) {
                            SI1(p);
                        } else {
                            // First loop is false, and we've got an else statement, execute it.
                            if (initial_loop && CNT(p) > 2) {
                                SI2(p);
                            }
                            break;
                        }
                        initial_loop = 0;
                    }

                    RETURN_SNODE_NULL();

                case T_IF:
                    node1 = SI0(p);
                    obj1 = si_get_object(node1);

                    // Check if it's already a boolean. If not, cast this object to boolean
                    if (! OBJECT_IS_BOOLEAN(obj1)) {
                        obj1 = object_call(obj1, "boolean", 0);
                    }

                    if (obj1 == Object_True) {
                        node2 = SI1(p);
                    } else if (CNT(p) > 2) {
                        node2 = SI2(p);
                    }

                    break;

                case T_METHOD_CALL :
//                    obj = SI0(p);
//                    method = SI1(p);
//                    // @TODO: add Arguments
//                    return object_call(obj, method, 0);
                    break;
//
                case T_FQMN :
                    if (CNT(p) > 1) {
                        saffire_error("FQMN can only have 1 operand");
                    }
                    node1 = SI0(p);
                    return node1;
//                    for (int i=0; i!=CNT(p); i++) {
//                        p->opr.ops[i];
//                    }
                    break;

                case '<' :
                        return si_cmp(p, COMPARISON_LT);
                        break;
                case '>' :
                        return si_cmp(p, COMPARISON_GT);
                        break;
                case T_GE :
                        return si_cmp(p, COMPARISON_GE);
                        break;
                case T_LE :
                        return si_cmp(p, COMPARISON_LE);
                        break;
                case T_NE :
                        return si_cmp(p, COMPARISON_NE);
                        break;
                case T_EQ :
                        return si_cmp(p, COMPARISON_EQ);
                        break;

                case '+' :
                        return si_opr(p, OPERATOR_ADD);
                        break;
                case '-' :
                        return si_opr(p, OPERATOR_SUB);
                        break;
                case '*' :
                        return si_opr(p, OPERATOR_MUL);
                        break;
                case '/' :
                        return si_opr(p, OPERATOR_DIV);
                        break;
                case T_AND :
                        return si_opr(p, OPERATOR_AND);
                        break;
                case T_OR :
                        return si_opr(p, OPERATOR_OR);
                        break;
                case '^' :
                        return si_opr(p, OPERATOR_XOR);
                        break;
                case T_SHIFT_LEFT :
                        return si_opr(p, OPERATOR_SHL);
                        break;
                case T_SHIFT_RIGHT :
                        return si_opr(p, OPERATOR_SHR);
                        break;

                default:
                    printf("Unhandled opcode: %d\n", p->opr.oper);
                    exit(1);
                    break;
            }
            break;
    }
    RETURN_SNODE_NULL();
}

/**
 * Compare the objects according to the comparison (returns 0 or 1)
 */
static t_snode *si_cmp(t_ast_element *p, int cmp) {
    t_snode *node1 = SI0(p);
    t_snode *node2 = SI1(p);

    // Check if the references are to the same object and we are doing a ==. If so, we are always true
    if (node1->data == node2->data && cmp == COMPARISON_EQ) return 0;

    t_object *obj1 = si_get_object(node1);
    t_object *obj2 = si_get_object(node2);
    if (obj1->type != obj2->type) {
        saffire_error("Types on comparison are not equal");
    }

    t_object *obj = object_comparison(obj1, cmp, obj2);
    RETURN_SNODE_OBJECT(obj);
}



/**
 * Calls object's operator
 */
static t_snode *si_opr(t_ast_element *p, int opr) {
    t_snode *node1 = SI0(p);
    t_snode *node2 = SI1(p);

    t_object *obj1 = si_get_object(node1);
    t_object *obj2 = si_get_object(node2);
    if (obj1->type != obj2->type) {
        saffire_error("Types on operator are not equal");
    }

    t_object *obj = object_operator(obj1, opr, 0, 1, obj2);
    RETURN_SNODE_OBJECT(obj);
}



#ifdef __DEBUG
    extern t_dll *object_dll;
#endif



/**
 *
 */
void saffire_interpreter(t_ast_element *p) {
    vars = ht_create();
    _saffire_interpreter(p);
    ht_destroy(vars);

#ifdef __DEBUG
    t_dll_element *e = DLL_HEAD(object_dll);
    while (e) {
        t_object *obj = (t_object *)e->data;
        printf("Object: %20s (%08X) Refcount: %d : %s \n", obj->name, obj, obj->ref_count, object_debug(obj));
        e = DLL_NEXT(e);
    }
#endif
}
