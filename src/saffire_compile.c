#include <stdio.h>
#include "saffire_compile.h"
#include "parser.tab.h"

static int lbl;

void saffire_compile(nodeType *p) {
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
                    saffire_compile(p->opr.ops[0]);
                    printf("\tjz\tL%03d\n", lbl2 = lbl++);
                    saffire_compile(p->opr.ops[1]);
                    printf("\tjmp\tL%03d\n", lbl1);
                    printf("L%03d:\n", lbl2);
                    break;

                case T_IF:
                    saffire_compile(p->opr.ops[0]);
                    if (p->opr.nops > 2) {
                        // if else
                        printf("\tjz\tL%03d\n", lbl1 = lbl++);
                        saffire_compile(p->opr.ops[1]);
                        printf("\tjmp\tL%03d\n", lbl2 = lbl++);
                        printf("L%03d:\n", lbl1);
                        saffire_compile(p->opr.ops[2]);
                        printf("L%03d\n", lbl2);
                    } else {
                        // if
                        printf("\tjz\tL%03d\n", lbl1 = lbl++);
                        saffire_compile(p->opr.ops[1]);
                        printf("L%03d:\n", lbl1);
                    }
                    break;

                case T_PRINT :
                    saffire_compile(p->opr.ops[0]);
                    printf("\tprint\n");
                    break;

                case '=' :
                    saffire_compile(p->opr.ops[1]);
                    printf("\tpop\t~\"%s\"\n", p->opr.ops[0]->var.name);
                    break;

                default :
                    saffire_compile(p->opr.ops[0]);
                    saffire_compile(p->opr.ops[1]);
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
                                printf("\n");
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