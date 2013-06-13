/*
 Copyright (c) 2012-2013, The Saffire Group
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
#include <stdlib.h>
#include <string.h>
#include "general/output.h"
#include "compiler/parser.tab.h"
#include "objects/attrib.h"
#include "objects/callable.h"
#include "compiler/ast_nodes.h"
#include "general/smm.h"
#include "general/path_handling.h"

extern char *get_token_string(int token);

static int node_nr = 0;



/**
 * Returns a list of flag strings for nice output in the graphs
 */
static char *show_modifiers(int modifiers) {
    char *s = (char *)smm_malloc(sizeof(char) * 100); // 100 bytes should be enough for everyone
    bzero(s, 100);

    if (modifiers & CALLABLE_FLAG_FINAL) s = strcat(s, "FINAL\\n");
    if (modifiers & CALLABLE_FLAG_ABSTRACT) s = strcat(s, "ABSTRACT\\n");
    if (modifiers & CALLABLE_FLAG_STATIC) s = strcat(s, "STATIC\\n");

    return s;
}

static char *show_attr_visibility(char visibility) {
    char *s = (char *)smm_malloc(sizeof(char) * 100); // 100 bytes should be enough for everyone
    bzero(s, 100);
    switch (visibility) {
        case ATTRIB_VISIBILITY_PUBLIC :
            snprintf(s, 99, "PUBLIC");
            break;
        case ATTRIB_VISIBILITY_PROTECTED :
            snprintf(s, 99, "PROTECTED");
            break;
        case ATTRIB_VISIBILITY_PRIVATE :
            snprintf(s, 99, "PRIVATE");
            break;
    }
    return s;
}

static char *show_attr_access(char access) {
    char *s = (char *)smm_malloc(sizeof(char) * 100); // 100 bytes should be enough for everyone
    bzero(s, 100);
    switch (access) {
        case ATTRIB_ACCESS_RW :
            snprintf(s, 99, "READ/WRITE");
            break;
        case ATTRIB_ACCESS_RO :
            snprintf(s, 99, "READONLY");
            break;
    }
    return s;
}


/**
 * Sanitize escape characters to _ so dot/graphiz does not complain
 */
char sanitized_string[100];
char *sanitize(char *s) {
    strncpy(sanitized_string, s, 99);
    for (int i=0; i!=strlen(sanitized_string); i++) {
        if (sanitized_string[i] < 32) sanitized_string[i] = '_';
    }
    return sanitized_string;
}


/**
 * Output node (and link to parent node number). Recursively called when child nodes are present.
 */
static void dot_node_iterate(FILE *fp, t_ast_element *p, int link_node_nr) {
    if (! p) {
        return;
    }

    // Store node_nr, since this is a static var that will change.
    int cur_node_nr = node_nr;
    node_nr++;

    fprintf(fp, "\tN_%d [", cur_node_nr);
    switch (p->type) {
        case typeAstString :
            fprintf(fp, "fillcolor=cornsilk2,style=\"filled, rounded\",label=\"{%d (#%ld)|Type=String|Value=\\\"%s\\\"}\"]\n", cur_node_nr, p->lineno, sanitize(p->string.value));
            break;

        case typeAstNumerical :
            fprintf(fp, "fillcolor=cornsilk2,style=\"filled, rounded\",label=\"{%d (#%ld)|Type=Numerical|Value=%d}\"]\n", cur_node_nr, p->lineno, p->numerical.value);
            break;

        case typeAstIdentifier :
            fprintf(fp, "fillcolor=darkolivegreen1,style=\"filled\",label=\"{%d (#%ld)|Type=Variable|Value=\\\"%s\\\"}\"]\n", cur_node_nr, p->lineno, p->identifier.name);
            break;

        case typeAstBool :
            fprintf(fp, "fillcolor=burlywood2,style=\"filled\",label=\"{%d (#%ld)|Type=Bool|Operator=%s} \"]\n", cur_node_nr, p->lineno, p->boolop.op ? "AND" : "OR");
            dot_node_iterate(fp, p->boolop.l, cur_node_nr);
            dot_node_iterate(fp, p->boolop.r, cur_node_nr);
            break;

        case typeAstAssignment :
            fprintf(fp, "fillcolor=burlywood2,style=\"filled\",label=\"{%d (#%ld)|Type=Assignment|Operator=%s (%d)} \"]\n", cur_node_nr, p->lineno, get_token_string(p->assignment.op), p->assignment.op);
            dot_node_iterate(fp, p->assignment.l, cur_node_nr);
            dot_node_iterate(fp, p->assignment.r, cur_node_nr);
            break;

        case typeAstProperty :
            fprintf(fp, "fillcolor=burlywood2,style=\"filled\",label=\"{%d (#%ld)|Type=Property} \"]\n", cur_node_nr, p->lineno);
            dot_node_iterate(fp, p->property.class, cur_node_nr);
            dot_node_iterate(fp, p->property.property, cur_node_nr);
            break;


        case typeAstComparison :
            fprintf(fp, "fillcolor=burlywood2,style=\"filled\",label=\"{%d (#%ld)|Type=Comparison|Operator=%s (%d)} \"]\n", cur_node_nr, p->lineno, get_token_string(p->comparison.cmp), p->comparison.cmp);
            dot_node_iterate(fp, p->comparison.l, cur_node_nr);
            dot_node_iterate(fp, p->comparison.r, cur_node_nr);
            break;

        case typeAstOperator:
            fprintf(fp, "fillcolor=burlywood1,style=\"filled\",label=\"{%d (#%ld)|Type=Operator|Operator=%s (%d)} \"]\n", cur_node_nr, p->lineno, get_token_string(p->operator.op), p->operator.op);
            dot_node_iterate(fp, p->operator.l, cur_node_nr);
            dot_node_iterate(fp, p->operator.r, cur_node_nr);
            break;

        case typeAstGroup :
            fprintf(fp, "label=\"{%d (#%ld)|Type=Group|Length=%d} \"]\n", cur_node_nr, p->lineno, p->group.len);

            // Plot all the items in the group
            for (int i=0; i!=p->group.len; i++) {
                dot_node_iterate(fp, p->group.items[i], cur_node_nr);
            }
            break;

        case typeAstOpr :
            fprintf(fp, "label=\"{%d (#%ld)|Type=Opr|Operator=%s (%d)| NrOps=%d} \"]\n", cur_node_nr, p->lineno, get_token_string(p->opr.oper), p->opr.oper, p->opr.nops);

            // Plot all the operands
            for (int i=0; i!=p->opr.nops; i++) {
                dot_node_iterate(fp, p->opr.ops[i], cur_node_nr);
            }
            break;

        case typeAstNull :
            fprintf(fp, "fillcolor=darkslategray1,style=\"filled, rounded\",label=\"{%d (#%ld)|Type=NULL}\"]\n", cur_node_nr, p->lineno);
            break;

        case typeAstNop :
            fprintf(fp, "fillcolor=darkslategray1,style=\"filled, rounded\",label=\"{%d (#%ld)|Type=NOP}\"]\n", cur_node_nr, p->lineno);
            break;

        case typeAstInterface :
            fprintf(fp, "fillcolor=darkseagreen,style=\"filled\",label=\"{%d (#%ld)|Type=Interface|Name=%s|Modifiers=%s (%d)}\"]\n", cur_node_nr, p->lineno, p->interface.name, show_modifiers(p->interface.modifiers), p->interface.modifiers);
            // Plot implementations and body
            dot_node_iterate(fp, p->interface.implements, cur_node_nr);
            dot_node_iterate(fp, p->interface.body, cur_node_nr);
            break;

        case typeAstClass :
            fprintf(fp, "fillcolor=darksalmon,style=\"filled\",label=\"{%d (#%ld)|Type=Class|Name=%s|Modifiers=%s (%d)}\"]\n", cur_node_nr, p->lineno, p->class.name, show_modifiers(p->class.modifiers), p->class.modifiers);
            // Plot body
            dot_node_iterate(fp, p->class.extends, cur_node_nr);
            dot_node_iterate(fp, p->class.implements, cur_node_nr);
            dot_node_iterate(fp, p->class.body, cur_node_nr);
            break;

        case typeAstAttribute:
            switch(p->attribute.attrib_type) {
                case ATTRIB_TYPE_METHOD :
                    fprintf(fp, "fillcolor=lightskyblue,style=\"filled\",label=\"{%d (#%ld)|Type=Attribute (Method)|Name=%s|Access=%s|Visiblity=%s|method_flags=%s}\"]\n",
                        cur_node_nr, p->lineno, p->attribute.name, show_attr_access(p->attribute.access), show_attr_visibility(p->attribute.visibility), show_modifiers(p->attribute.method_flags));

                    // Plot arguments
                    dot_node_iterate(fp, p->attribute.arguments, cur_node_nr);
                    break;
                case ATTRIB_TYPE_PROPERTY :
                    fprintf(fp, "fillcolor=lightskyblue,style=\"filled\",label=\"{%d (#%ld)|Type=Attribute (Property)|Name=%s|Access=%s|Visiblity=%s}\"]\n",
                        cur_node_nr, p->lineno, p->attribute.name, show_attr_access(p->attribute.access), show_attr_visibility(p->attribute.visibility));
                    break;
                case ATTRIB_TYPE_CONSTANT :
                    fprintf(fp, "fillcolor=lightskyblue,style=\"filled\",label=\"{%d (#%ld)|Type=Attribute (Constant)|Name=%s|Access=%s|Visiblity=%s}\"]\n",
                        cur_node_nr, p->lineno, p->attribute.name, show_attr_access(p->attribute.access), show_attr_visibility(p->attribute.visibility));
                    break;
            }
            dot_node_iterate(fp, p->attribute.value, cur_node_nr);
            break;

        default :
            fprintf(fp, "style=\"filled,rounded\",fillcolor=firebrick1,label=\"{%d (#%ld)|Type=UNKNOWN|Value=%d}\"]\n", node_nr, p->lineno, p->type);
            break;
    }

    // This node is connected to the linked node nr (unless it's the root-node)
    if (link_node_nr != -1) {
        fprintf(fp, "\tN_%d -> N_%d\n", link_node_nr, cur_node_nr);
    }
}

void dot_generate(t_ast_element *ast, const char *outputfile) {
    FILE *fp = fopen(outputfile, "w");
    if (!fp) {
        fatal_error(1, "Cannot open %s for writing\n", outputfile);
        return;
    }

    char *png_file = replace_extension(outputfile, ".dot", ".png");
    fprintf(fp, "# Generated by dot_generate(). Generate with: dot -T png -o %s %s\n", png_file, outputfile);
    smm_free(png_file);
    fprintf(fp, "digraph G {\n");
    fprintf(fp, "\tnode [ shape = record ];\n");
    fprintf(fp, "\n");

    dot_node_iterate(fp, ast, -1);

    fprintf(fp, "}\n");
    fclose(fp);
}

