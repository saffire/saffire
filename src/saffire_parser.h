#ifndef __SAFFIRE_PARSER_H__
#define __SAFFIRE_PARSER_H__

    #include "node.h"

    nodeType *saffire_strCon(char *value);
    nodeType *saffire_intCon(int value);
    nodeType *saffire_var(char *var_name);
    nodeType *saffire_opr(int opr, int nops, ...);
    void saffire_free_node(nodeType *p);
    void saffire_execute(nodeType *p);

#endif