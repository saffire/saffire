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
#include "../compiler/parser.tab.h"
#include "compiler/saffire_compiler.h"
#include "general/hashtable.h"
#include "compiler/ast.h"
#include "general/stack.h"
#include "general/smm.h"
#include "objects/object.h"
#include "objects/method.h"
#include "objects/base.h"
#include "objects/string.h"
#include "objects/numerical.h"
#include "objects/null.h"
#include "objects/boolean.h"
#include "debug.h"

extern char *get_token_string(int token);
static t_snode *_saffire_interpreter(t_ast_element *p);

// A stack maintained by the AST which holds the current line for the current AST
t_dll *lineno_stack;

extern char *wctou8(const wchar_t *wstr, long len);
#define OBJ2STR(_obj_) wctou8(((t_string_object *)_obj_)->value, ((t_string_object *)_obj_)->char_length)


t_stack *scope_stack;

t_scope *get_current_scope(void) {
    t_dll_element *e = DLL_TAIL(scope_stack->dll);
    return e->data;
}
void leave_scope(void) {
    t_scope *last_scope = stack_pop(scope_stack);
    DEBUG_PRINT(">>>>>>> Leaving scope level %d\n", last_scope->depth);
    smm_free(last_scope);
}
void enter_scope(t_ast_element *p) {
    t_scope *current_scope = get_current_scope();

    t_scope *new_scope = smm_malloc(sizeof(t_scope));
    new_scope->context = si_get_current_context();
    new_scope->entrypoint = p;
    new_scope->depth = current_scope->depth + 1;
    stack_push(scope_stack, new_scope);

    DEBUG_PRINT(">>>>>>> Entering scope level %d\n", new_scope->depth);
}

extern t_hash_table *object_hash;

/**
 *
 */
static void si_init(void) {
    // Create stack for linenumbers
    lineno_stack = dll_init();

    // Initialize scope
    scope_stack = stack_init();

    // Add global scope
    t_scope *tmp = smm_malloc(sizeof(t_scope));
    tmp->context = NULL;
    tmp->depth = 0;
    tmp->entrypoint = NULL;
    stack_push(scope_stack, tmp);
}


/**
 *
 */
static void si_fini(void) {
    // @TODO: Something is wrong with freeing this DLL :(
    //dll_free(lineno_stack);

    stack_free(scope_stack);
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
        char *key = node->data.id.id;
        obj = si_find_var_in_context(key, NULL);
        if (! obj) {
            saffire_error("This variable is not initialized!");
        }
        return obj;
    }

    saffire_error("This identifier does not have any ID nor OBJ");
    return NULL;
}


/**
 * Sets or replaces the object into the variable
 */
static void si_set_object(t_snode *node, t_object *dst_obj) {
    t_object *src_obj = NULL;

    if (! IS_IDENTIFIER(node)) {
        saffire_error("Trying to set an object to a non-variable");
    }

    // Decrease source object reference count (if any object is present)
    if (HAS_IDENTIFIER_OBJ(node)) {
        src_obj = si_get_object(node);
        if (src_obj != dst_obj) {
            // Only decrease when they are not equal (ie: when changing objects)
            object_dec_ref(src_obj);
        }
    }

    // Add or replace the object
    si_create_var_in_context(node->data.id.id, NULL, dst_obj, CTX_CREATE_OR_UPDATE);

    if (src_obj != dst_obj) {
        // Only increase when they are not equal (ie: when changed)
        object_inc_ref(dst_obj);
    }
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



// Pointer to the current object
t_object *current_obj = NULL;





struct _object *object_user_new(t_object *obj, va_list arg_list) {
    DEBUG_PRINT("object_create_new_instance called");

    t_object *new_obj = smm_malloc(sizeof(t_object));
    memcpy(new_obj, obj, sizeof(t_object));

    // Reset refcount for new object
    new_obj->ref_count = 0;

    // These are instances
    new_obj->flags &= ~OBJECT_TYPE_MASK;
    new_obj->flags |= OBJECT_TYPE_INSTANCE;

    return new_obj;
}

#ifdef __DEBUG
char global_buf[1024];
static char *object_user_debug(struct _object *obj) {
    sprintf(global_buf, "User object[%s]", obj->name);
    return global_buf;
}
#endif

// String object management functions
t_object_funcs user_funcs = {
        object_user_new,              // Allocate a new string object
        NULL,             // Free a string object
        NULL,                 // Clone a string object
#ifdef __DEBUG
        object_user_debug
#endif
};



/**
 *
 */
static t_snode *_saffire_interpreter(t_ast_element *p) {
    t_object *obj, *obj1, *obj2, *obj3;
    t_snode *node1, *node2, *node3;
    int initial_loop;
    t_ast_element *hte;
    char *ctx_name, *name;
    wchar_t *wchar_tmp;
    t_dll *dll;
    t_scope *scope;

    // Append to lineno
    dll_append(lineno_stack, (void *)p->lineno);

    // No element found, return NULL object
    if (!p) {
        RETURN_SNODE_OBJECT(Object_Null);
    }


    switch (p->type) {
        case typeAstNull :
            RETURN_SNODE_NULL();
            break;
        case typeAstInterface:
            // @TODO
            RETURN_SNODE_NULL();
            break;
        case typeAstString :
            DEBUG_PRINT("new string object: '%s'\n", p->string.value);

            // Allocate enough room to hold string in wchar and convert
            int len = strlen(p->string.value) * sizeof(wchar_t);
            wchar_tmp = (wchar_t *)smm_malloc(len * sizeof(wchar_t));
            memset(wchar_tmp, 0, len * sizeof(wchar_t));
            mbstowcs(wchar_tmp, p->string.value, strlen(p->string.value));

            // create string object
            obj = object_new(Object_String, wchar_tmp);

            // Free tmp wide string
            smm_free(wchar_tmp);
            RETURN_SNODE_OBJECT(obj);
            break;

        case typeAstNumerical :
            DEBUG_PRINT("new numerical object: %d\n", p->numerical.value);
            // Create numerical object
            obj = object_new(Object_Numerical, p->numerical.value);
            RETURN_SNODE_OBJECT(obj);
            break;

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

            obj = si_find_var_in_context(p->string.value, NULL);
            RETURN_SNODE_IDENTIFIER(p->string.value, obj);
            break;

        case typeAstClass :
            obj = (t_object *)smm_malloc(sizeof(t_object));
            obj->ref_count = 0;
            obj->type = objectTypeAny;
            obj->name = smm_strdup(p->class.name);
            obj->flags = OBJECT_TYPE_CLASS;
            obj->parent = NULL;
            obj->implement_count = 0;
            obj->implements = NULL;
            obj->methods = ht_create();
            obj->properties = ht_create();
            obj->constants = ht_create();
            obj->operators = NULL;
            obj->comparisons = NULL;
            obj->funcs = &user_funcs;

            // @TODO: Add modifier flags to obj->flags


            // Check extends
            obj->parent = Object_Base;

            // Interpret body.
            t_object *saved_obj = current_obj;
            current_obj = obj;
            _saffire_interpreter(p->class.body);
            current_obj = saved_obj;

            // Add the object to the current context
            t_ns_context *ctx = si_get_current_context();
            si_context_add_object(ctx, obj);
            break;

        case typeAstMethod :
            if (current_obj == NULL) {
                saffire_error("Trying to define a method outside a class. This should be caught by the parser!");
            }
            DEBUG_PRINT("Adding method: %s to %s\n", p->method.name, current_obj->name);

            // @TODO: ADD FLAGS AND VISIBILITY
            int vis = 0;
            if (p->method.modifiers & MODIFIER_PUBLIC) vis |= METHOD_VISIBILITY_PUBLIC;
            if (p->method.modifiers & MODIFIER_PROTECTED) vis |= METHOD_VISIBILITY_PROTECTED;
            if (p->method.modifiers & MODIFIER_PRIVATE) vis |= METHOD_VISIBILITY_PRIVATE;

            int flags = 0;
            if (p->method.modifiers & MODIFIER_FINAL) flags |= METHOD_FLAG_FINAL;
            if (p->method.modifiers & MODIFIER_ABSTRACT) flags |= METHOD_FLAG_ABSTRACT;
            if (p->method.modifiers & MODIFIER_STATIC) flags |= METHOD_FLAG_STATIC;

            object_add_external_method(current_obj, p->method.name, flags, vis, p->method.body);
            break;

        case typeAstOpr :
            DEBUG_PRINT("opr.oper: %s(%d)\n", get_token_string(p->opr.oper), p->opr.oper);
            switch (p->opr.oper) {
                case T_PROGRAM :
                    SI0(p); // parse use declarations
                    SI1(p); // parse top statements
                    break;
                case T_TOP_STATEMENTS:
                case T_USE_STATEMENTS:
                case T_STATEMENTS :
                    for (int i=0; i!=OP_CNT(p); i++) {
                        _saffire_interpreter(p->opr.ops[i]);
                    }
                    // Statements do not return anything
                    RETURN_SNODE_NULL(); // (well, it should)
                    break;

                case T_IMPORT :
                    // Class to import
                    node1 = SI0(p);
                    obj1 = si_get_object(node1);
                    char *classname = OBJ2STR(obj1);

                    // Fetch alias
                    node2 = SI1(p);
                    if (IS_NULL(node2)) {
                        node2 = node1;  // Alias X as X if needed
                    }
                    obj2 = si_get_object(node2);
                    char *alias = OBJ2STR(obj2);

                    // context to import from
                    node3 = SI2(p);
                    obj3 = si_get_object(node3);
                    ctx_name = OBJ2STR(obj3);


                    // Check if variable is free
                    if (si_find_var_in_context(alias, NULL)) {
                        saffire_error("A variable named %s is already present or imported.", alias);
                    }

                    // Find class in context
                    ctx = si_get_context(ctx_name);
                    if (ctx == NULL) {
                        saffire_error("Cannot find context: %s", ctx_name);
                    }
                    obj = si_find_var_in_context(classname, ctx);
                    if (! obj) {
                        saffire_error("Cannot find class %s inside context: %s", classname, ctx_name);
                    }

                    // Add the object to the current context as the alias variable
                    si_create_var_in_context(alias, NULL, obj, CTX_CREATE_ONLY);

                    DEBUG_PRINT("Imported class %s as %s from %s into %s\n", classname, alias, ctx_name, ctx->name);
                    break;
                case T_USE :
                    // Class to use
                    node1 = SI0(p);
                    obj1 = si_get_object(node1);
                    name = OBJ2STR(obj1);

                    if (OP_CNT(p) > 1) {
                        // Fetch alias
                        node2 = SI1(p);
                        if (IS_NULL(node2)) {
                            node2 = node1;  // Alias X as X
                        }
                    } else {
                        node2 = node1;  // Alias X as X
                    }
                    obj2 = si_get_object(node2);
                    alias = OBJ2STR(obj2);

                    // Check if variable is free
                    ctx = si_find_context(alias);
                    if (ctx != NULL) {
                        saffire_error("A context named %s is already present or imported.", alias);
                    }

                    ctx = si_find_context(name);
                    if (ctx == NULL) {
                        saffire_error("Context %s was not found.", name);
                    }

                    si_create_context_alias(alias, ctx);
                    break;

                case T_RETURN :
                    // Check the current scope.
                    scope = get_current_scope();
                    if (scope->depth == 1) {
                        DEBUG_PRINT("Cannot leave the global scope!");
                    }
                    
                    node1 = SI0(p);
                    obj = si_get_object(node1);
                    RETURN_SNODE_OBJECT(obj);
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

                    // it should be a variable, otherwise we cannot write to it..

                    // @TODO: THIS IS WRONG. AN ID ALWAYS HAS AN KEY?
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
                            obj2 = object_find_method(obj1, "boolean");
                            obj1 = object_call(obj1, obj2, 0);
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
                            obj2 = object_find_method(obj1, "boolean");
                            obj1 = object_call(obj1, obj2, 0);
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
                            obj2 = object_find_method(obj1, "boolean");
                            obj1 = object_call(obj1, obj2, 0);
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
                        obj2 = object_find_method(obj1, "boolean");
                        obj1 = object_call(obj1, obj2, 0);
                    }

                    if (obj1 == Object_True) {
                        // Execute if-block
                        node2 = SI1(p);
                    } else if (OP_CNT(p) > 2) {
                        // Execute (optional) else-block
                        node2 = SI2(p);
                    }
                    break;

                case T_ARGUMENT_LIST:
                    dll = dll_init();
                    for (int i=0; i!=OP_CNT(p); i++) {
                        node1 = _saffire_interpreter(p->opr.ops[i]);
                        obj1 = si_get_object(node1);
                        dll_append(dll, obj1);
                    }
                    RETURN_SNODE_DLL(dll);

                case T_METHOD_CALL :
                    // Get object
                    node1 = SI0(p);
                    obj1 = IS_NULL(node1) ? NULL : si_get_object(node1);

                    if (obj1 != NULL) {
                        hte = p->opr.ops[1];
                        obj2 = object_find_method(obj1,  hte->identifier.name);
                        if (! obj2) {
                            saffire_error("Cannot find method or property named '%s' in '%s'", hte->identifier.name, obj1->name);
                        }
                    } else {
                        // Get object
                        node2 = SI1(p);
                        obj2 = si_get_object(node2);
                    }



                    // Get arguments (or NULL)
                    t_dll *dll = NULL;
                    node2 = SI2(p);
                    if (IS_DLL(node2)) {
                        dll = node2->data.dll;
                    } else if (IS_NULL(node2)) {
                        dll = NULL;
                    } else {
                        saffire_error("Expected a DLL (or null)");
                    }

                    // assume nothing found
                    obj3 = Object_Null;

                    if (OBJECT_IS_METHOD(obj2)) {
                        /*
                         * Lots of method checks before we can actually call this
                         */
                        t_method_object *method = (t_method_object *)obj2;
                        if (METHOD_IS_CONSTRUCTOR(method)) {
                            saffire_error("Cannot call constructor");
                        }
                        if (METHOD_IS_DESTRUCTOR(method)) {
                            saffire_error("Cannot call destructor");
                        }
                        if (OBJECT_TYPE_IS_ABSTRACT(obj1)) {
                            saffire_error("Cannot call an abstract class");
                        }
                        if (OBJECT_TYPE_IS_INTERFACE(obj1)) {
                            saffire_error("Cannot call an interface");
                        }

                        if (OBJECT_TYPE_IS_INSTANCE(obj1) && METHOD_IS_STATIC(method)) {
                            saffire_error("Cannot call a static method from an instance. Hint: use %s.%s()", obj1->name, obj2->name);
                        }
                        if (OBJECT_TYPE_IS_CLASS(obj1) && ! METHOD_IS_STATIC(method)) {
                            saffire_error("Cannot call a non-static method directly from a class. Hint: instantiate first");
                        }


                        // Set new scope
                        enter_scope(p);

                        // We need to do a method call
                        DEBUG_PRINT("+++ Calling method %s \n", obj2->name);
                        obj3 = object_call_args(obj1, obj2, dll);

                        leave_scope();

                    } else if (OBJECT_TYPE_IS_CLASS(obj2)) {
                        // We need to instantiate
                        DEBUG_PRINT("+++ Instantiating a new class for %s\n", obj2->name);

                        enter_scope(p);

                        obj3 = object_new(obj2, dll);
                        if (! obj3) {
                            saffire_error("Cannot instantiate class %s", obj2->name);
                        }

                        leave_scope();
                    } else {
                        saffire_error("Cannot call or instantiate %s", obj2->name);
                    }

//                    } else {
//
//                        // get class or method name
//                        hte = p->opr.ops[1];
//                        if (hte->type != typeAstIdentifier) {
//                            saffire_error("Can only have identifiers here", hte->identifier.name);
//                        }
//
//                        method_name = smm_strdup(hte->identifier.name);
//                    }
//
//                    // At this point we need to fetch the objec,
//
//
//
////                    if (hte->type == typeAstNull && (obj->flags & OBJECT_TYPE_CLASS) == OBJECT_TYPE_CLASS) {
//
//                    if (instantiation) {
//                        // Instantiating
//                        DEBUG_PRINT("+++ Instantiating a new class for %s\n", obj1->name);
//
//                        if (! OBJECT_TYPE_IS_CLASS(obj1)) {
//                            saffire_error("Can only instantiate classes");
//                        }
//
//                        obj2 = object_new(obj1, dll);
//                        if (! obj2) {
//                            saffire_error("Cannot instantiate class %s", obj1->name);
//                        }
//
//                    } else {
//
//                        if (hte->type != typeAstIdentifier) {
//                            saffire_error("Can only have identifiers here", hte->identifier.name);
//                        }
//                        method_name = smm_strdup(hte->identifier.name);
//
//                        obj2 = object_call_args(obj1, method_name, dll);
//
//                        smm_free(method_name);
//                    }


                    if (dll) dll_free(dll);

                    RETURN_SNODE_OBJECT(obj3);
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

                case '.' :
                    node1 = SI0(p);
                    obj1 = si_get_object(node1);

                    // get method name from object
                    hte = p->opr.ops[1];
                    if (hte->type != typeAstIdentifier) {
                        saffire_error("Can only have identifiers here", hte->identifier.name);
                    }

                    DEBUG_PRINT("Figuring out: '%s' in object '%s'\n", hte->identifier.name, obj1->name);
                    obj = ht_find(obj1->properties, hte->identifier.name);
                    if (obj == NULL) {
                        obj = ht_find(obj1->constants, hte->identifier.name);
                        if (obj == NULL) {
                            saffire_error("Cannot find constant or property '%s' from '%s'", hte->identifier.name, obj1->name);
                        }
                    }
                    RETURN_SNODE_OBJECT(obj);

                    break;

                case T_CONST :
                    if (current_obj == NULL) {
                        // @TODO: We could create constants OUTSIDE a class!
                        saffire_error("Defining constants outside classes is not yet supported!");
                    }

                    hte = p->opr.ops[0];
                    if (hte->type != typeAstIdentifier) {
                        saffire_error("Constant name needs to be an identifier");
                    }

                    node2 = SI1(p);
                    obj2 = si_get_object(node2);

                    DEBUG_PRINT("Added constant %s to %s\n", hte->identifier.name, current_obj->name);
                    ht_add(current_obj->constants, hte->identifier.name, obj2);
                    break;

                case T_PROPERTY :
                    if (current_obj == NULL) {
                        saffire_error("Cannot define properties outside classes. This should be caught by the parser!");
                    }

                    hte = p->opr.ops[0];
                    if (hte->type != typeAstNumerical) {
                        saffire_error("Flags name needs to be numerical");
                    }

                    hte = p->opr.ops[1];
                    if (hte->type != typeAstIdentifier) {
                        saffire_error("Property name needs to be an identifier");
                    }

                    node2 = SI2(p);
                    obj2 = si_get_object(node2);

                    DEBUG_PRINT("Added property %s to %s\n", hte->identifier.name, current_obj->name);
                    ht_add(current_obj->properties, hte->identifier.name, obj2);
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
 * Interpret a leaf. Returns the last object encountered, or a NULL object.
 */
t_object *saffire_interpreter_leaf(t_ast_element *p) {
    t_snode *node = _saffire_interpreter(p);
    if (IS_OBJECT(node)) {
        RETURN_OBJECT(node->data.obj);
    }

    RETURN_NULL;
}


/**
 * Interpret a tree. Will deal with initialiazation etc
 */
int saffire_interpreter(t_ast_element *p) {
    int ret = 0;

    si_init();

    t_object *obj = saffire_interpreter_leaf(p);

    if (OBJECT_IS_NUMERICAL(obj)) {
        ret = ((t_numerical_object *)(obj))->value;
    }

    // @TODO: What should we do with the output of this node? Somehow, return it to the caller or somethign?

#ifdef __DEBUG
    t_hash_iter iter;
    ht_iter_rewind(&iter, object_hash);
    while (ht_iter_valid(&iter)) {
        t_object *obj = ht_iter_value(&iter);
        if (obj->type != objectTypeCode && obj->type != objectTypeMethod) {
            DEBUG_PRINT("Object: %20s (%08X) Refcount: %d : %s \n", obj->name, obj, obj->ref_count, object_debug(obj));
        }
        ht_iter_next(&iter);
    }
#endif

    si_fini();

    return ret;
}
