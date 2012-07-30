#include <stdio.h>
#include <stdlib.h>
#include "saffire_compiler.h"
#include "parser.tab.h"

static int lbl;

#define SC0(p) saffire_compiler(p->opr.ops[0])
#define SC1(p) saffire_compiler(p->opr.ops[1])
#define SC2(p) saffire_compiler(p->opr.ops[2])


void saffire_compiler(nodeType *p) {
    int lbl1, lbl2;

    if (!p) return;

    switch (p->type) {
        case typeStrCon :
            printf("\tpush\t\"%s\"\n", p->strCon.value);
            break;

        case typeIntCon :
            printf("\tpush\t%d\n", p->intCon.value);
            break;

        case typeVar :
            printf("\tpush\t~\"%s\"\n", p->var.name);
            break;

        case typeOpr :
            switch (p->opr.oper) {
                case T_WHILE :
                    printf("L%03d:\n", lbl1 = lbl++);
                    SC0(p);
                    printf("\tjz\tL%03d\n", lbl2 = lbl++);
                    SC1(p);
                    printf("\tjmp\tL%03d\n", lbl1);
                    printf("L%03d:\n", lbl2);
                    break;

                case T_IF:
                    SC0(p);
                    if (p->opr.nops > 2) {
                        // if else
                        printf("\tjz\tL%03d\n", lbl1 = lbl++);
                        SC1(p);
                        printf("\tjmp\tL%03d\n", lbl2 = lbl++);
                        printf("L%03d:\n", lbl1);
                        SC2(p);
                        printf("L%03d:\n", lbl2);
                    } else {
                        // if
                        printf("\tjz\tL%03d\n", lbl1 = lbl++);
                        SC1(p);
                        printf("L%03d:\n", lbl1);
                    }
                    break;

                case T_PRINT :
                    SC0(p);
                    printf("\tprint\n");
                    break;

                case '=' :
                    SC1(p);
                    printf("\tpop\t~\"%s\"\n", p->opr.ops[0]->var.name);
                    break;

                default :
                    SC0(p);
                    SC1(p);
                    switch (p->opr.oper) {
                        case '+' : printf("\tadd\n"); break;
                        case '-' : printf("\tsub\n"); break;
                        case '*' : printf("\tmul\n"); break;
                        case '/' : printf("\tdiv\n"); break;
                        case '^' : printf("\tpow\n"); break;
                        case '<' : printf("\tcompLT\n"); break;
                        case '>' : printf("\tcompGT\n"); break;
                        case T_GE : printf("\tcompGE\n"); break;
                        case T_LE : printf("\tcompLE\n"); break;
                        case T_NE : printf("\tcompNE\n"); break;
                        case T_EQ : printf("\tcompEQ\n"); break;
                        case T_INC : printf("\tinc\n"); break;
                        case T_DEC : printf("\tdec\n"); break;
                        case ';' :
                                // End of statement?
                                break;
                        default:
                            printf("Unhandled opcode: %d\n", p->opr.oper);
                            exit(1);
                            break;
                    }
            }
    }
}