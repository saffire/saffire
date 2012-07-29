#ifndef __NODE_H__
#define __NODE_H__

    #include "svar.h"

    // different kind of nodes we manage
    typedef enum { typeStrCon, typeIntCon, typeVar, typeOpr } nodeEnum;

    typedef struct {
        char *value;                // Pointer to the actual constant string
    } strConNodeType;

    typedef struct {
        int value;                  // Integer constant
    } intConNodeType;

    typedef struct {
        char *name;                 // Name of the actual variable to use
    } varNodeType;

    typedef struct {
        int oper;                   // Operator
        int nops;                   // number of additional operands
        struct nodeTypeTag **ops;   // Operands
    } oprNodeType;

    typedef struct nodeTypeTag {
        nodeEnum type;              // Type of the node
        union {
            intConNodeType intCon;        // constant int
            strConNodeType strCon;        // constant string
            varNodeType var;              // variable
            oprNodeType opr;              // operator
        };
    } nodeType;

#endif