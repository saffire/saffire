#ifndef __NODE_H__
#define __NODE_H__

    #include "svar.h"

    // different kind of nodes we manage
    typedef enum { TypeVar, typeOpr } nodeEnum;

    // A variable node points to a var
    typedef struct {
        nodeEnum type;              // Type of the node
        svar *var;                  // pointer to svar variable
    } varNodeType;

    typedef struct {
        nodeEnum type;              // type of the node
        int oper;                   // Operator
        int nops;                   // number of additional operands
        struct nodeTypeTag **ops;   // Operands
    } oprNodeType;

    typedef struct nodeTypeTag {
        nodeEnum type;              // Type of the node
        union {
            varNodeType var;        // variable
            oprNodeType opr;        // operator
        };
    } nodeType;

#endif