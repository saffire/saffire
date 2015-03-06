/*
 Copyright (c) 2012-2015, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
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
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <saffire/general/output.h>
#include <saffire/compiler/parser_helpers.h>
#include <saffire/compiler/saffire_parser.h>
#include <saffire/general/smm.h>
#include <saffire/compiler/ast_nodes.h>
#include <saffire/vm/context.h>
#include <saffire/debug.h>
#include "parser.tab.h"

/**
 * Allocate a new element (untyped) ast_node element
 */
static t_ast_element *ast_node_alloc_element(void) {
    t_ast_element *p;

    p = smm_malloc(sizeof(t_ast_element));
    bzero(p, sizeof(t_ast_element));

    return p;
}


/**
 * Boolean comparison node: left AND,OR right
 */
t_ast_element *ast_node_boolop(int lineno, int boolop, t_ast_element *left, t_ast_element *right) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstBool;
    p->boolop.op = boolop;
    p->boolop.l = left;
    p->boolop.r = right;

    return p;
}

/**
 * Assignment node: "left =,+=,/=,.. right"
 */
t_ast_element *ast_node_assignment(int lineno, int op, t_ast_element *left, t_ast_element *right) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstAssignment;
    p->assignment.op = op;
    p->assignment.l = left;
    p->assignment.r = right;

    return p;
}
/**
 * Comparison node: "left <,>,<=,>= right"
 */
t_ast_element *ast_node_comparison(int lineno, int cmp, t_ast_element *left, t_ast_element *right) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstComparison;
    p->comparison.cmp = cmp;
    p->comparison.l = left;
    p->comparison.r = right;

    return p;
}

/**
 * Operator node: "left +,-,/,shl,shr right"
 */
t_ast_element *ast_node_operator(int lineno, int op, t_ast_element *left, t_ast_element *right) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstOperator;
    p->operator.op = op;
    p->operator.l = left;
    p->operator.r = right;

    return p;
}

/**
 * String node
 */
t_ast_element *ast_node_string(int lineno, char *value) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstString;
    p->string.value = string_strdup0(value);

    return p;
}

t_ast_element *ast_node_regex(int lineno, char *value) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstRegex;
    p->regex.value = string_strdup0(value);

    return p;
}


t_ast_element *ast_node_id_to_string(t_ast_element *src) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = src->lineno;
    p->type = typeAstString;
    p->string.value = string_strdup0(src->identifier.name);

    return p;
}

/**
 * Duplicate AST string node
 */
t_ast_element *ast_node_string_dup(int lineno, t_ast_element *src) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstString;
    p->string.value = string_strdup0(src->string.value);

    return p;
}

t_ast_element *ast_node_string_context_class(int lineno, char *identifier) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstString;
    p->string.value = vm_context_get_class(identifier);

    return p;
}

/**
 * Creates a numerical node
 */
t_ast_element *ast_node_numerical(int lineno, int value) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstNumerical;
    p->numerical.value = value;

    return p;
}

/**
 * Creates a identifier node
 */
t_ast_element *ast_node_identifier(int lineno, char *var_name) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstIdentifier;
    p->identifier.name = string_strdup0(var_name);
    return p;
}

/**
 * Creates a property node
 */
t_ast_element *ast_node_property(int lineno, t_ast_element *class, t_ast_element *property) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstProperty;
    p->property.class = class;
    p->property.property = property;

    return p;
}

/**
 * Creates a nop node. This does not generate any code. It's just for filling AST-trees so they are in sync (for instance: if/then/nop if/then/else)
 */
t_ast_element *ast_node_nop(void) {
    t_ast_element *p = ast_node_alloc_element();

    p->type = typeAstNop;
    return p;
}

/**
 * Creates a identifier node with "null" node.
 */
t_ast_element *ast_node_null(int lineno) {
    t_ast_element *p = ast_node_alloc_element();

    p->type = typeAstNull;
    p->lineno = lineno;
    return p;
}



t_ast_element *ast_node_add_multi(t_ast_element *src, t_ast_element *group) {
    if (group->grouping == 0) {
        fatal_error(1, "Cannot add a non-grouping element to group");   /* LCOV_EXCL_LINE */
    }

    switch (group->type) {
        // Tuples and groups are processed the same way
        case typeAstGroup :
        case typeAstTuple :
            // Resize memory
            src->group.items = smm_realloc(src->group.items, (src->group.len + group->group.len) * sizeof(t_ast_element));
            if (src->group.items == NULL) {
                fatal_error(1, "Out of memory");   /* LCOV_EXCL_LINE */
            }

            // Add new operator
            for (int i=0; i!=group->group.len; i++) {
                src->group.items[src->group.len] = group->group.items[i];
                src->group.len++;
            }
            break;
        case typeAstOpr :
            // Resize memory
            src->opr.ops = smm_realloc(src->opr.ops, (src->opr.nops + group->opr.nops) * sizeof(t_ast_element));
            if (src->opr.ops == NULL) {
                fatal_error(1, "Out of memory");   /* LCOV_EXCL_LINE */
            }

            // Add new operator
            for (int i=0; i!=group->opr.nops; i++) {
                src->opr.ops[src->opr.nops] = group->opr.ops[i];
                src->opr.nops++;
            }
            break;

        default :
            fatal_error(1, "unhandled group type");   /* LCOV_EXCL_LINE */
            break;
    }

    return src;
}
/**
 * Add a node to an existing operator node. This allows to add children later inside a tree (like lists)
 */
t_ast_element *ast_node_add(t_ast_element *src, t_ast_element *new_element) {
    if (src->grouping == 0) {
        fatal_error(1, "Cannot add to non-grouping element");   /* LCOV_EXCL_LINE */
    }

    switch (src->type) {
        // Tuples and groups are processed the same way
        case typeAstGroup :
        case typeAstTuple :
            // Resize memory
            src->group.items = smm_realloc(src->group.items, (src->group.len + 1) * sizeof(t_ast_element));
            if (src->group.items == NULL) {
                fatal_error(1, "Out of memory");   /* LCOV_EXCL_LINE */
            }

            // Add new operator
            src->group.items[src->group.len] = new_element;
            src->group.len++;
            break;
        case typeAstOpr :
            // Resize memory
            src->opr.ops = smm_realloc(src->opr.ops, (src->opr.nops+1) * sizeof(t_ast_element));
            if (src->opr.ops == NULL) {
                fatal_error(1, "Out of memory");   /* LCOV_EXCL_LINE */
            }

            // Add new operator
            src->opr.ops[src->opr.nops] = new_element;
            src->opr.nops++;
            break;

        default :
            fatal_error(1, "unhandled group type");   /* LCOV_EXCL_LINE */
            break;
    }

    return src;
}


/**
 * Create a group node. This can contain 0 or more initial elements.
 */
t_ast_element *ast_node_group(int len, ...) {
    t_ast_element *p = ast_node_alloc_element();
    va_list ap;

    p->type = typeAstGroup;
    p->grouping = 1;
    p->group.len = len;
    p->group.items = NULL;

    // Add additional nodes (they can be added later with ast_add())
    if (len) {
        p->group.items = smm_malloc (len * sizeof(t_ast_element));
        va_start(ap, len);
        for (int i=0; i < len; i++) {
            p->group.items[i] = va_arg(ap, t_ast_element *);
        }
        va_end(ap);
    }

    return p;
}

t_ast_element *ast_node_tuple(int len, ...) {
    t_ast_element *p = ast_node_alloc_element();
    va_list ap;

    p->type = typeAstTuple;
    p->grouping = 1;
    p->group.len = len;
    p->group.items = NULL;

    // Add additional nodes (they can be added later with ast_add())
    if (len) {
        p->group.items = smm_malloc (len * sizeof(t_ast_element));
        va_start(ap, len);
        for (int i=0; i < len; i++) {
            p->group.items[i] = va_arg(ap, t_ast_element *);
        }
        va_end(ap);
    }

    return p;
}


/**
 * Create an operator node with 0 or more operands. More operands can be added later through ast_node_add()
 *
 * Note: this is not the same as ast_node_operator(), which generated +-/& operator codes.
 */
t_ast_element *ast_node_opr(int lineno, int opr, int nops, ...) {
    t_ast_element *p = ast_node_alloc_element();
    va_list ap;

    p->lineno = lineno;
    p->type = typeAstOpr;
    p->grouping = 1;
    p->opr.oper = opr;
    p->opr.nops = nops;
    p->opr.ops = NULL;

    // Add additional nodes (they can be added later with ast_add())
    if (nops) {
        p->opr.ops = smm_malloc (nops * sizeof(t_ast_element));
        va_start(ap, nops);
        for (int i=0; i < nops; i++) {
            p->opr.ops[i] = va_arg(ap, t_ast_element *);
        }
        va_end(ap);
    }

    return p;
}


/**
 * Concatenates an identifier node onto an existing identifier node
 */
t_ast_element *ast_node_identifier_concat(t_ast_element *src, char *s) {
    src->identifier.name= smm_realloc(src->identifier.name, strlen(src->identifier.name) + strlen(s) + 1);
    strcat(src->identifier.name, s);
    return src;
}

/**
 * Concatenates an string node onto an existing string node
 */
t_ast_element *ast_node_string_concat(t_ast_element *src, char *s) {
    src->string.value = smm_realloc(src->string.value, strlen(src->string.value) + strlen(s) + 1);
    strcat(src->string.value, s);
    return src;
}

/**
 * Create a class node.
 */
t_ast_element *ast_node_class(int lineno, t_class *class, t_ast_element *body) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstClass;
    p->class.modifiers = class->modifiers;
    p->class.name = string_strdup0(class->name);

    p->class.extends = class->extends;
    p->class.implements = class->implements;

    p->class.body = body;

    return p;
}

/**
 * Create an interface node
 */
t_ast_element *ast_node_interface(int lineno, int modifiers, char *name, t_ast_element *implements, t_ast_element *body) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstInterface;
    p->interface.modifiers = modifiers;
    p->interface.name = string_strdup0(name);
    p->interface.implements = implements;
    p->interface.body = body;

    return p;
}

/**
 * Create a attribute node
 */
t_ast_element *ast_node_attribute(int lineno, char *name, char attrib_type, char visibility, char access, t_ast_element *value, char method_flags, t_ast_element *arguments) {
    t_ast_element *p = ast_node_alloc_element();

    p->lineno = lineno;
    p->type = typeAstAttribute;
    p->attribute.name = string_strdup0(name);
    p->attribute.attrib_type = attrib_type;
    p->attribute.visibility = visibility;
    p->attribute.access = access;
    p->attribute.value = value;
    p->attribute.method_flags = method_flags;
    p->attribute.arguments = arguments;

    return p;
}


/**
 * Free up an AST node
 */
void ast_free_node(t_ast_element *p) {
    if (!p) return;

    switch (p->type) {
        case typeAstNop :
        case typeAstNull :
        case typeAstNumerical :
            // Nothing to free
            break;
        case typeAstAttribute :
            smm_free(p->attribute.name);
            ast_free_node(p->attribute.value);
            ast_free_node(p->attribute.arguments);
            break;
        case typeAstProperty :
            ast_free_node(p->property.class);
            ast_free_node(p->property.property);
            break;
        case typeAstOperator :
            ast_free_node(p->operator.r);
            ast_free_node(p->operator.l);
            break;
        case typeAstBool :
            ast_free_node(p->boolop.r);
            ast_free_node(p->boolop.l);
            break;
        case typeAstAssignment :
            ast_free_node(p->assignment.r);
            ast_free_node(p->assignment.l);
            break;
        case typeAstComparison:
            ast_free_node(p->comparison.r);
            ast_free_node(p->comparison.l);
            break;
        case typeAstString :
            smm_free(p->string.value);
            break;
        case typeAstRegex :
            smm_free(p->regex.value);
            break;
        case typeAstIdentifier :
            smm_free(p->identifier.name);
            break;
        case typeAstClass :
            smm_free(p->class.name);
            ast_free_node(p->class.extends);
            ast_free_node(p->class.implements);
            ast_free_node(p->class.body);
            break;
        case typeAstInterface :
            smm_free(p->interface.name);
            ast_free_node(p->interface.implements);
            ast_free_node(p->interface.body);
            break;
        case typeAstOpr :
            if (p->opr.nops) {
                for (int i=0; i < p->opr.nops; i++) {
                    ast_free_node(p->opr.ops[i]);
                }
                smm_free(p->opr.ops);
            }
            break;
        case typeAstTuple :
        case typeAstGroup :
            if (p->group.len) {
                for (int i=0; i < p->group.len; i++) {
                    ast_free_node(p->group.items[i]);
                }
                smm_free(p->group.items);
            }
            break;
    }
    smm_free(p);
}

int yyparse (yyscan_t scanner, SaffireParser *saffireParser);

/**
 * Compiles a file into an AST (through bison). Returns the AST root node.
 */
t_ast_element *ast_generate_tree(FILE *fp, char *filename) {
    /*
     * Init parser structures
     */
    SaffireParser sp;
    yyscan_t scanner;

    // Initialize saffire structure
    sp.file = fp;
    sp.filename = filename;
    sp.eof = 0;
    sp.ast = NULL;
    sp.error = NULL;
    sp.yyparse = NULL;
    sp.yyparse_args = NULL;
    sp.yyexec = NULL;
    sp.parserinfo = alloc_parserinfo();

    // Initialize scanner structure and hook the saffire structure as extra info
    yylex_init_extra(&sp, &scanner);

    //int status = yyparse(scanner, &sp);
    int status = yyparse(scanner, &sp);
    if (status == 1) {
        sp.ast = NULL;
    }
    t_ast_element *ast = sp.ast;

    // Since we've done the complete file, we don't need anything
    free_parserinfo(sp.parserinfo);
    sp.parserinfo = NULL;

    yylex_destroy(scanner);

    return ast;
}

/**
 * Generate an AST from a source file
 */
t_ast_element *ast_generate_from_file(const char *source_file) {
    FILE *fp;
    char *fp_name;

    // Open file, or use stdin if needed
    if (! strcmp(source_file, "-")) {
        fp = stdin;
        fp_name = "<stdin>";
    } else {
        fp = fopen(source_file, "r");
        fp_name = (char *)source_file;
    }
    if (!fp) {
        fatal_error(1, "Could not open file: %s\n", source_file);   /* LCOV_EXCL_LINE */
    }

    // Generate source file into an AST tree
    t_ast_element *ast = ast_generate_tree(fp, fp_name);

    // Close file
    fclose(fp);

    return ast;
}
