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
#include "interpreter/context.h"
#include "interpreter/errors.h"
#include "compiler/parser.tab.h"
#include "general/hashtable.h"
#include "compiler/ast.h"
#include "general/smm.h"
#include "object/object.h"
#include "object/string.h"
#include "object/numerical.h"
#include "object/null.h"
#include "object/boolean.h"
#include "debug.h"

extern char *get_token_string(int token);
extern char *wctou8(const wchar_t *wstr, long len);
static t_snode *_saffire_interpreter(t_ast_element *p);

// A stack maintained by the AST which holds the current line for the current AST
t_dll *lineno_stack;

#ifdef __DEBUG
extern t_dll *object_dll;
#endif


/**
 *
 */
static void si_init(void) {
    // Create stack for linenumbers
    lineno_stack = dll_init();

    context_init();
}


/**
 *
 */
static void si_fini(void) {
    context_fini();

    // @TODO: Something is wrong with freeing this DLL :(
    //dll_free(lineno_stack);
}





/**
 * Returns object from a node. If the node points to a variable, return the object where this variable
 * points to.
 *
 * $a    -> returns the object where $a points to
 * <obj> -> returns plain object
 */
static t_object *si_get_object(t_snode *node) {
    t_object *obj;

    if (IS_OBJECT(node)) {
        return node->data.obj;
    }

    if (HAS_IDENTIFIER_OBJ(node)) {
        // Fetch object where this var references to, and add it to the destination var
        return node->data.id.obj;
    }

    if (HAS_IDENTIFIER_ID(node)) {
        // Fetch object where this var references to, and add it to the destination var
        //t_hash_table_bucket *htb = si_find_in_context(node->data.id.id);
        t_hash_table_bucket *htb = node->data.id.id;
        obj = (t_object *)htb->data;
        if (obj == NULL) {
            saffire_error("This variable is not initialized!");
        }
        return obj;
    }

    saffire_error("This identifier does not have any ID nor OBJ");
}


/**
 * Sets or replaces the object into the variable
 */
static void si_set_object(t_snode *node, t_object *dst_obj) {
    if (! IS_IDENTIFIER(node)) {
        saffire_error("Trying to set an object to a non-variable");
    }

    // Decrease source object reference count (if any object is present)
    if (HAS_IDENTIFIER_OBJ(node)) {
        t_object *src_obj = si_get_object(node);
        object_dec_ref(src_obj);
    }

    // Set new object and increase reference count
    t_hash_table_bucket *htb = node->data.id.id;
    htb->data = dst_obj;
    object_inc_ref(dst_obj);
}

/**
 * Compare the objects according to the comparison (returns 0 or 1)
 */
static t_snode *si_comparison(t_ast_element *p, int cmp) {
    t_snode *node1 = SI0(p);
    t_snode *node2 = SI1(p);

    // Check if the references are to the same object and we are doing a ==. If so, we are always true
    if (IS_OBJECT(node1) && IS_OBJECT(node2) && cmp == COMPARISON_EQ && node1->data.obj == node2->data.obj) return 0;

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
static t_snode *si_operator(t_ast_element *p, int opr) {
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



/**
 *
 */
static t_snode *_saffire_interpreter(t_ast_element *p) {
    t_object *obj, *obj1, *obj2, *obj3;
    t_snode *node1, *node2, *node3;
    t_hash_table_bucket *htb;
    int ret, initial_loop, len;
    t_ast_element *hte;
    char *str, *method_name;

    // Append to lineno
    dll_append(lineno_stack, (void *)p->lineno);

    if (!p) {
        RETURN_SNODE_OBJECT(Object_Null);
    }

    wchar_t *wchar_tmp;
    switch (p->type) {
        case typeAstString :
            DEBUG_PRINT("new string object: '%s'\n", p->string.value);

            // Allocate enough room to hold string in wchar and convert
            int len = strlen(p->string.value)*sizeof(wchar_t);
            wchar_tmp = (wchar_t *)smm_malloc(len * sizeof(wchar_t));
            memset(wchar_tmp, 0, len * sizeof(wchar_t));
            mbstowcs(wchar_tmp, p->string.value, strlen(p->string.value));

            // create string object
            obj = object_new(Object_String, wchar_tmp);

            // Free tmp wide string
            smm_free(wchar_tmp);
            RETURN_SNODE_OBJECT(obj);

        case typeAstNumerical :
            DEBUG_PRINT("new numerical object: %d\n", p->numerical.value);
            // Create numerical object
            obj = object_new(Object_Numerical, p->numerical.value);
            RETURN_SNODE_OBJECT(obj);

        case typeAstIdentifier :
            // Do constant vars
            if (strcasecmp(p->string.value, "True") == 0) {
                //RETURN_SNODE_OBJECT(Object_True);
                RETURN_SNODE_IDENTIFIER(NULL, Object_True);
            }
            if (strcasecmp(p->string.value, "False") == 0) {
                DEBUG_PRINT("Retuning false!");
                //RETURN_SNODE_OBJECT(Object_False);
                RETURN_SNODE_IDENTIFIER(NULL, Object_False);
            }
            if (strcasecmp(p->string.value, "Null") == 0) {
                //RETURN_SNODE_OBJECT(Object_Null);
                RETURN_SNODE_IDENTIFIER(NULL, Object_Null);
            }

            htb = si_find_in_context(p->string.value);
            RETURN_SNODE_IDENTIFIER(htb, (t_object *)htb->data);
            break;

        case typeAstOpr :
            DEBUG_PRINT("opr.oper: %s(%d)\n", get_token_string(p->opr.oper), p->opr.oper);
            switch (p->opr.oper) {
                case T_PROGRAM :
                    SI0(p); // use declarations
                    SI1(p); // top statements
                    break;
                case T_TOP_STATEMENTS:
                case T_USE_STATEMENTS:
                case T_STATEMENTS :
                    for (int i=0; i!=OP_CNT(p); i++) {
                        _saffire_interpreter(p->opr.ops[i]);
                    }
                    // Statements do not return anything
                    RETURN_SNODE_NULL();
                    break;
                case T_USE :
                    node1 = SI0(p);
                    obj1 = si_get_object(node1);

                    char *namespace = wctou8(((t_string_object *)obj1)->value, ((t_string_object *)obj1)->char_length);
                    if (OP_CNT(p) == 2) {
                        node2 = SI1(p);
                        obj2 = si_get_object(node2);
                        namespace = wctou8(((t_string_object *)obj2)->value, ((t_string_object *)obj2)->char_length);
                    }

                    si_create_context(namespace);
                    break;

                case T_EXPRESSIONS :
                    // No expression, just return NULL
                    if (OP_CNT(p) == 0) {
                        RETURN_SNODE_NULL();
                    }

                    // Do all expressions
                    for (int i=0; i!=OP_CNT(p); i++) {
                        node1 = _saffire_interpreter(p->opr.ops[i]);
                        // Remember the first node
                        if (i == 0) node2 = node1;
                    }
                    return node2;
                    break;

                case T_ASSIGNMENT :
                    // Fetch LHS node
                    node1 = SI0(p);

                    // Not found, create this variable
                    if (node1 == NULL) {
                        saffire_error("Left hand side does not exist");
                    }
                    // it should be a variable, otherwise we cannot write to it..
                    if (! HAS_IDENTIFIER_ID(node1)) {
                        saffire_error("Left hand side is not writable!");
                    }

                    // Check if we have a normal assignment. We only support this for now...
                    t_ast_element *e = p->opr.ops[1];
                    if (e->type != typeAstOpr || e->opr.oper != T_ASSIGNMENT) {
                        saffire_error("We only support = assignments (no += etc)");
                    }

                    // Evaluate the RHS
                    node3 = SI2(p);

                    // Get the object and store it
                    obj1 = si_get_object(node3);
                    si_set_object(node1, obj1);

                    RETURN_SNODE_OBJECT(obj1);
                    break;

                /**
                 * Control structures
                 */
                case T_DO :
                    do {
                        // Always execute our inner block at least once
                        SI0(p);

                        // Check condition
                        node1 = SI1(p);
                        obj1 = si_get_object(node1);
                        // Check if it's already a boolean. If not, cast this object to boolean
                        if (! OBJECT_IS_BOOLEAN(obj1)) {
                            obj1 = object_call(obj1, "boolean", 0);
                        }

                        // False, we can break our do-loop
                        if (obj1 == Object_False) {
                            break;
                        }
                    } while (1);

                    RETURN_SNODE_NULL();
                    break;

                case T_WHILE :
                    initial_loop = 1;
                    while (1) {
                        // Check condition first
                        node1 = SI0(p);
                        obj1 = si_get_object(node1);
                        // Check if it's already a boolean. If not, cast this object to boolean
                        if (! OBJECT_IS_BOOLEAN(obj1)) {
                            obj1 = object_call(obj1, "boolean", 0);
                        }

                        // if condition is true, execute our inner block
                        if (obj1 == Object_True) {
                            SI1(p);
                        } else {
                            // If the first loop is false and we've got an else statement, execute it.
                            if (initial_loop && OP_CNT(p) > 2) {
                                SI2(p);
                            }
                            break;
                        }

                        initial_loop = 0;
                    }

                    RETURN_SNODE_NULL();
                    break;

                case T_FOR :
                    // Evaluate first part
                    node1 = SI0(p);

                    while (1) {
                        // Check condition first
                        node2 = SI1(p);
                        obj1 = si_get_object(node2);
                        // Check if it's already a boolean. If not, cast this object to boolean
                        if (! OBJECT_IS_BOOLEAN(obj1)) {
                            obj1 = object_call(obj1, "boolean", 0);
                        }

                        // if condition is not true, break our loop
                        if (obj1 != Object_True) {
                            break;
                        }

                        // Condition is true, execute our inner loop
                        SI3(p);

                        // Finally, evaluate our last block
                        SI2(p);
                    }

                    // All done
                    break;

                /**
                 * Conditional statements
                 */
                case T_IF:
                    node1 = SI0(p);
                    obj1 = si_get_object(node1);

                    // Check if it's already a boolean. If not, cast this object to boolean
                    if (! OBJECT_IS_BOOLEAN(obj1)) {
                        obj1 = object_call(obj1, "boolean", 0);
                    }

                    if (obj1 == Object_True) {
                        // Execute if-block
                        node2 = SI1(p);
                    } else if (OP_CNT(p) > 2) {
                        // Execute (optional) else-block
                        node2 = SI2(p);
                    }
                    break;

                case T_METHOD_CALL :
                    node1 = SI0(p);
                    obj1 = si_get_object(node1);

                    hte = p->opr.ops[1];
                    if (hte->type != typeAstIdentifier) {
                        saffire_error("Can only have identifiers here", hte->identifier.name);
                    }

                    if (hte->identifier.name[0] == '$') {
                        obj2 = (t_object *)htb->data;
                        if (! OBJECT_IS_STRING(obj2)) {
                            saffire_error("This variable does not point to a string object: '%s'", hte->identifier.name);
                        }
                        method_name = wctou8(((t_string_object *)obj2)->value, ((t_string_object *)obj2)->char_length);
                    } else {
                        // We duplicate, so we can always free the string later on
                        method_name = smm_strdup(hte->identifier.name);
                    }

                    // @TODO: add Arguments
                    obj2 = object_call(obj1, method_name, 0);

                    smm_free(method_name);

                    RETURN_SNODE_OBJECT(obj2);
                    break;

                /* Comparisons */
                case '<' :
                    return si_comparison(p, COMPARISON_LT);
                    break;
                case '>' :
                    return si_comparison(p, COMPARISON_GT);
                    break;
                case T_GE :
                    return si_comparison(p, COMPARISON_GE);
                    break;
                case T_LE :
                    return si_comparison(p, COMPARISON_LE);
                    break;
                case T_NE :
                    return si_comparison(p, COMPARISON_NE);
                    break;
                case T_EQ :
                    return si_comparison(p, COMPARISON_EQ);
                    break;

                /* Operators */
                case '+' :
                    return si_operator(p, OPERATOR_ADD);
                    break;
                case '-' :
                    return si_operator(p, OPERATOR_SUB);
                    break;
                case '*' :
                    return si_operator(p, OPERATOR_MUL);
                    break;
                case '/' :
                    return si_operator(p, OPERATOR_DIV);
                    break;
                case T_AND :
                    return si_operator(p, OPERATOR_AND);
                    break;
                case T_OR :
                    return si_operator(p, OPERATOR_OR);
                    break;
                case '^' :
                    return si_operator(p, OPERATOR_XOR);
                    break;
                case T_SHIFT_LEFT :
                    return si_operator(p, OPERATOR_SHL);
                    break;
                case T_SHIFT_RIGHT :
                    return si_operator(p, OPERATOR_SHR);
                    break;

                /* Unary operators */
                case T_OP_INC :
                    // We must be a variable
                    node1 = SI0(p);
                    if (! HAS_IDENTIFIER_ID(node1)) {
                        saffire_error("Left hand side is not writable!");
                    }

                    obj1 = si_get_object(node1);
                    obj2 = object_new(Object_Numerical, 1);
                    obj3 = object_operator(obj1, OPERATOR_ADD, 0, 1, obj2);

                    si_set_object(node1, obj3);

                    RETURN_SNODE_OBJECT(obj3);
                    break;
                case T_OP_DEC :
                    // We must be a variable
                    node1 = SI0(p);
                    if (! HAS_IDENTIFIER_ID(node1)) {
                        saffire_error("Left hand side is not writable!");
                    }

                    obj1 = si_get_object(node1);
                    obj2 = object_new(Object_Numerical, 1);
                    obj3 = object_operator(obj1, OPERATOR_SUB, 0, 1, obj2);

                    si_set_object(node1, obj3);

                    RETURN_SNODE_OBJECT(obj3);
                    break;


                default:
                    saffire_error("Unhandled opcode: %d\n", p->opr.oper);
                    break;
            }
            break;
    }
    RETURN_SNODE_NULL();
}



/**
 *
 */
void saffire_interpreter(t_ast_element *p) {
    si_init();
    _saffire_interpreter(p);

#ifdef __DEBUG
    t_dll_element *e = DLL_HEAD(object_dll);
    while (e) {
        t_object *obj = (t_object *)e->data;
        DEBUG_PRINT("Object: %20s (%08X) Refcount: %d : %s \n", obj->name, obj, obj->ref_count, object_debug(obj));
        e = DLL_NEXT(e);
    }
#endif

    si_fini();
}
