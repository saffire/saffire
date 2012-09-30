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
#include "object/null.h"
#include "object/boolean.h"

#define SI(p)   (_saffire_interpreter(p))
#define SI0(p)  (_saffire_interpreter(p->opr.ops[0]))
#define SI1(p)  (_saffire_interpreter(p->opr.ops[1]))
#define SI2(p)  (_saffire_interpreter(p->opr.ops[2]))

static int obj_idx = 0;

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
    exit(1);
}



#define SNODE_NULL          0
#define SNODE_OBJECT        1
#define SNODE_METHOD        2
#define SNODE_VARIABLE      3

typedef struct _snode {
    char    type;
    void    *data;
} t_snode;


#define AF_READABLE 1           // Item is readable (usable for RHS)
#define AF_WRITABLE 2           // Item is writable (usable for LHS)

//
//static int svar_idx = 0;
//
//svar *svar_temp_alloc(int type, char *s, long l) {
//    char str[512];
//    sprintf(str, "TMP-%d", svar_idx++);
//    svar *tmp = svar_alloc(type, str, s, l);
//    svar_print(tmp);
//    return tmp;
//}

#define IS_OBJECT(snode)  snode.type == SNODE_OBJECT
#define IS_METHOD(snode)  snode.type == SNODE_METHOD
#define IS_VARIABLE(snode)  snode.type == SNODE_VARIABLE


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
t_object *si_obj(t_ast_element *p, int idx) {
    if (idx > p->opr.nops) {
        saffire_warning("Warning, want index %d, but max is %d\n", idx, p->opr.nops);
    }

    t_snode *snode = p->opr.ops[idx];
    if (snode->type != SNODE_OBJECT) {
        saffire_warning("Warning, need an object, but got a %d\n", snode->type);
    }
    return snode->data;
}


/**
 *
 */
t_snode *_saffire_interpreter(t_ast_element *p) {
    t_object *obj, *obj1, *obj2, *obj3;
    t_hash_table_bucket *htb;

    if (!p) {
        RETURN_SNODE_OBJECT(Object_Null);
    }

    printf ("interpreting(%d)\n", p->type);

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
            obj = object_new(Object_String, p->numerical.value);
            RETURN_SNODE_OBJECT(obj);

        case typeIdentifier :
            htb = ht_find(vars, p->string.value);
            if (htb == NULL) {
                ht_add(vars, p->string.value, Object_Null);    // create NULL var by default
                htb = ht_find(vars, p->string.value);
            }

            // Do constant vars
            obj = si_obj(p, 1);
            if (strcasecmp(obj->string.value, "True") == 0) {
                RETURN_SNODE_OBJECT(Object_True);
            }
            if (strcasecmp(obj->string.value, "False") == 0) {
                RETURN_SNODE_OBJECT(Object_False);
            }
            if (strcasecmp(obj->string.value, "Null") == 0) {
                RETURN_SNODE_OBJECT(Object_Null);
            }

            // Return variable, otherwise it's a method (name)
            if (p->string.value[0] == '$') {
                RETURN_SNODE_VARIABLE(htb->data);
            } else {
                RETURN_SNODE_METHOD(htb->data);
            }

        case typeOpr :
            printf ("opr.oper(%d)\n", p->opr.oper);
            switch (p->opr.oper) {
                case T_PROGRAM :
                    printf ("Use statements:\n");
                    si_obj(p, 0);
                    printf ("Actual app:\n");
                    si_obj(p, 1);
                    break;

                case T_TOP_STATEMENTS:
                case T_USE_STATEMENTS:
                case T_STATEMENTS :
                    for (int i=0; i!=p->opr.nops; i++) {
                        _saffire_interpreter(p->opr.ops[i]);
                    }
                    break;
                case T_USE :
                    // @TODO: Use statements are not operational
                    saffire_warning("Use statements are not functional");
                    break;

                case T_EXPRESSIONS :
                    SI0(p);
                    break;

                case T_ASSIGNMENT :
                    htb = (t_hash_table_bucket *)SI0(p);
                    if (strcmp(SI1(p)->str.value, "=") == 0) {
                        printf("Assignment is not =\n");
                        htb->data = SI2(p);
                    }
                    return htb->data;
                    break;

//                case T_WHILE :
//                    while (svar_true(SI0(p))) {
//                        SI1(p);
//                    }
//                    return svar_temp_alloc(SV_NULL, NULL, 0);
//
//                case T_IF:
//                    if (svar_true(SI0(p))) {
//                        SI1(p);
//                    } else if (p->opr.nops > 2) {
//                        SI2(p);
//                    }
//                    return svar_temp_alloc(SV_NULL, NULL, 0);

                case T_METHOD_CALL :
//                    obj = SI0(p);
//                    method = SI1(p);
//                    // @TODO: add Arguments
//                    return object_call(obj, method, 0);
                    break;
//
                case T_FQMN :
                    //tmp = SI0(p);
                    for (int i=0; i!=p->opr.nops; i++) {
                        p->opr.ops[i];
                    }
                    break;
//
//                case ';' :
//                    SI0(p);
//                    return SI1(p);
//                case '<' :
//                        var1 = SI0(p);
//                        var2 = SI0(p);
//                        i = (var1->val.l < var2->val.l);
//                        return svar_temp_alloc(SV_LONG, NULL, i);
//
//                case '>' :
//                        var1 = SI0(p);
//                        var2 = SI1(p);
//                        i = (var1->val.l > var2->val.l);
//                        return svar_temp_alloc(SV_LONG, NULL, i);
//
//                case T_GE :
//                        var1 = SI0(p);
//                        var2 = SI1(p);
//                        i = (var1->val.l >= var2->val.l);
//                        return svar_temp_alloc(SV_LONG, NULL, i);
//
//                case T_LE :
//                        var1 = SI0(p);
//                        var2 = SI1(p);
//                        i = (var1->val.l <= var2->val.l);
//                        return svar_temp_alloc(SV_LONG, NULL, i);
//
//                case T_NE :
//                        var1 = SI0(p);
//                        var2 = SI1(p);
//                        i = (var1->val.l != var2->val.l);
//                        return svar_temp_alloc(SV_LONG, NULL, i);
//
//                case T_EQ :
//                        var1 = SI0(p);
//                        var2 = SI1(p);
//                        i = (var1->val.l == var2->val.l);
//                        return svar_temp_alloc(SV_LONG, NULL, i);

                case '+' :
                          obj1 = SI0(p);
                          obj2 = SI1(p);
                          obj = object_call(obj1, "::add", 1, obj2);
                          RETURN_SNODE_OBJECT(obj);
                case '-' :
                          obj1 = SI0(p);
                          obj2 = SI1(p);
                          obj = object_call(obj1, "::sub", 1, obj2);
                          RETURN_SNODE_OBJECT(obj);
                case '*' :
                          obj1 = SI0(p);
                          obj2 = SI1(p);
                          obj = object_call(obj1, "::mul", 1, obj2);
                          RETURN_SNODE_OBJECT(obj);
                case '/' :
                          obj1 = SI0(p);
                          obj2 = SI1(p);
                          obj = object_call(obj1, "::div", 1, obj2);
                          RETURN_SNODE_OBJECT(obj);
                case T_AND :
                          obj1 = SI0(p);
                          obj2 = SI1(p);
                          obj = object_call(obj1, "::and", 1, obj2);
                          RETURN_SNODE_OBJECT(obj);
                case T_OR :
                          obj1 = SI0(p);
                          obj2 = SI1(p);
                          obj = object_call(obj1, "::or", 1, obj2);
                          RETURN_SNODE_OBJECT(obj);
                case '^' :
                          obj1 = SI0(p);
                          obj2 = SI1(p);
                          obj = object_call(obj1, "::xor", 1, obj2);
                          RETURN_SNODE_OBJECT(obj);

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
 *
 */
void saffire_interpreter(t_ast_element *p) {
    vars = ht_create();
    _saffire_interpreter(p);
    ht_destroy(vars);
//    return ret;
}
