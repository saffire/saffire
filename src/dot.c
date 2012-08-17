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
#include "parser.tab.h"
#include "ast.h"
#include <string.h>

extern char *get_token_string(int token);

static int node_nr = 0;



/**
 * Returns a list of flag strings for nice output in the graphs
 */
static char *show_modifiers(int modifiers) {
    char *s = (char *)calloc(100, sizeof(char)); // 100 bytes should be enough for everyone

    if (modifiers & CONST_CLASS_PROTECTED) s = strcat(s, "PROTECTED\\n");
    if (modifiers & CONST_CLASS_PUBLIC) s = strcat(s, "PUBLIC\\n");
    if (modifiers & CONST_CLASS_PRIVATE) s = strcat(s, "PRIVATE\\n");
    if (modifiers & CONST_CLASS_FINAL) s = strcat(s, "FINAL\\n");
    if (modifiers & CONST_CLASS_ABSTRACT) s = strcat(s, "ABSTRACT\\n");
    if (modifiers & CONST_CLASS_STATIC) s = strcat(s, "STATIC\\n");
    if (modifiers & CONST_CLASS_READONLY) s = strcat(s, "READONLY\\n");

    return s;
}


/**
 * Output node (and link to parent node number). Recursively called when child nodes are present.
 */
static void saffire_dot_node_iterate(FILE *fp, t_ast_element *p, int link_node_nr) {
    // Store node_nr, since this is a static var that will change.
    int cur_node_nr = node_nr;
    node_nr++;

    fprintf(fp, "\tN_%d [", cur_node_nr);
    switch (p->type) {
        case typeStrCon :
            fprintf(fp, "fillcolor=cornsilk2,style=\"filled, rounded\",label=\"{N:%d|Type=String|Value=\\\"%s\\\"}\"]\n", cur_node_nr, p->strCon.value);
            break;

        case typeIntCon :
            fprintf(fp, "fillcolor=cornsilk2,style=\"filled, rounded\",label=\"{N:%d|Type=Numerical|Value=%d}\"]\n", cur_node_nr, p->intCon.value);
            break;

        case typeVar :
            fprintf(fp, "fillcolor=darkolivegreen1,style=\"filled, rounded\",label=\"{N:%d|Type=Variable|Value=\\\"%s\\\"}\"]\n", cur_node_nr, p->var.name);
            break;

        case typeOpr :
            fprintf(fp, "label=\"{N:%d|Type=Opr|Operator=%s (%d)| NrOps=%d} \"]\n", cur_node_nr, get_token_string(p->opr.oper), p->opr.oper, p->opr.nops);

            // Plot all the operands
            for (int i=0; i!=p->opr.nops; i++) {
                saffire_dot_node_iterate(fp, p->opr.ops[i], cur_node_nr);
            }
            break;

        case typeNull :
            fprintf(fp, "fillcolor=darkslategray1,style=\"filled, rounded\",label=\"{N:%d|Type=NULL}\"]\n", cur_node_nr);
            break;

        case typeInterface :
            fprintf(fp, "fillcolor=darkseagreen,style=\"filled\",label=\"{N:%d|Type=Interface|Name=%s|Modifiers=%s (%d)}\"]\n", cur_node_nr, p->interface.name, show_modifiers(p->interface.modifiers), p->interface.modifiers);
            // Plot implementations and body
            saffire_dot_node_iterate(fp, p->interface.implements, cur_node_nr);
            saffire_dot_node_iterate(fp, p->interface.body, cur_node_nr);
            break;

        case typeClass :
            fprintf(fp, "fillcolor=darksalmon,style=\"filled\",label=\"{N:%d|Type=Class|Name=%s|Modifiers=%s (%d)}\"]\n", cur_node_nr, p->class.name, show_modifiers(p->class.modifiers), p->class.modifiers);
            // Plot extends, implementations and body
            saffire_dot_node_iterate(fp, p->class.extends, cur_node_nr);
            saffire_dot_node_iterate(fp, p->class.implements, cur_node_nr);
            saffire_dot_node_iterate(fp, p->class.body, cur_node_nr);
            break;

        case typeMethod:
            fprintf(fp, "fillcolor=lightskyblue,style=\"filled\",label=\"{N:%d|Type=Method|Name=%s|Modifiers=%s (%d)}\"]\n", cur_node_nr, p->method.name, show_modifiers(p->method.modifiers), p->method.modifiers);
            // Plot arguments and body
            saffire_dot_node_iterate(fp, p->method.arguments, cur_node_nr);
            saffire_dot_node_iterate(fp, p->method.body, cur_node_nr);
            break;

        default :
            fprintf(fp, "style=\"filled,rounded\",fillcolor=firebrick1,label=\"{N:%d|Type=UNKNOWN|Value=%d}\"]\n", node_nr, p->type);
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
        printf("Cannot open %s for writing\n", outputfile);
        return;
    }

    fprintf(fp, "# Generated by dot_generate(). Generate with: dot -T png -o %s.png %s\n", outputfile, outputfile);
    fprintf(fp, "digraph G {\n");
    fprintf(fp, "\tnode [ shape = record ];\n");
    fprintf(fp, "\n");

    saffire_dot_node_iterate(fp, ast, -1);

    fprintf(fp, "}\n");
    fclose(fp);
}

