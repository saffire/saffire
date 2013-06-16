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
#include <unistd.h>
#include <string.h>
#include "general/config.h"
#include "debugger/dbgp/xml.h"
#include "debugger/dbgp/args.h"
#include "debugger/dbgp/sock.h"
#include "debugger/dbgp/commands.h"
#include "debug.h"
#include "general/smm.h"

// Status names. Mapped
char *dbgp_status_names[5] = { "starting", "stopping", "stopped", "running", "break" };

struct dbgp_command dbgp_commands[] = {
    { "context_names", DBGP_CMD_REF(context_names) },
    { "context_get", DBGP_CMD_REF(context_get) },

    { "eval", DBGP_CMD_REF(context_names) },
    { "feature_set", DBGP_CMD_REF(context_names) },
    { "feature_get", DBGP_CMD_REF(context_names) },

    { "typemap_get", DBGP_CMD_REF(context_names) },
    { "property_get", DBGP_CMD_REF(context_names) },
    { "property_set", DBGP_CMD_REF(context_names) },
    { "property_value", DBGP_CMD_REF(context_names) },

    { "source", DBGP_CMD_REF(context_names) },
    { "stack_get", DBGP_CMD_REF(stack_get) },
    { "stack_depth", DBGP_CMD_REF(stack_get) },
    { "status", DBGP_CMD_REF(stack_get) },

    { "stderr", DBGP_CMD_REF(stack_get) },
    { "stdout", DBGP_CMD_REF(stack_get) },

    { "breakpoint_get", DBGP_CMD_REF(breakpoint_get) },
    { "breakpoint_set", DBGP_CMD_REF(breakpoint_set) },
    { "breakpoint_list", DBGP_CMD_REF(breakpoint_list) },
    { "breakpoint_remove", DBGP_CMD_REF(breakpoint_remove) },
    { "breakpoint_update", DBGP_CMD_REF(breakpoint_update) },

    { "step_into", &do_command_step_into },
    { "step_over", &do_command_step_over },
    { "step_out", &do_command_step_over },
    { "stop", &do_command_detach },
    { "detach", &do_command_detach },
    { "run", &do_command_run },
    { NULL, NULL }
};



/**
 *
 */
DBGP_CMD_DEF(context_names) {
    xmlNodePtr node;

    xmlNodePtr root_node = dbgp_xml_create_response("context_names", argc, argv);

    node = xmlNewChild(root_node, NULL, BAD_CAST "context", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "Locals");
    xmlNewProp(node, BAD_CAST "id", BAD_CAST "0");

    node = xmlNewChild(root_node, NULL, BAD_CAST "context", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "Globals");
    xmlNewProp(node, BAD_CAST "id", BAD_CAST "1");

    node = xmlNewChild(root_node, NULL, BAD_CAST "context", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "Class");
    xmlNewProp(node, BAD_CAST "id", BAD_CAST "2");


    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(step_into) {
    di->state = DBGP_STATE_RUNNING;

    xmlNodePtr root_node = dbgp_xml_create_response("step_into", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(step_over) {
    di->state = DBGP_STATE_RUNNING;

    xmlNodePtr root_node = dbgp_xml_create_response("step_over", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(stack_get) {
    xmlNodePtr node;

    // Check for -d depth

    xmlNodePtr root_node = dbgp_xml_create_response("stack_get", argc, argv);

//    node = xmlNewChild(root_node, NULL, BAD_CAST "stack_level", NULL);
//    xmlNewProp(node, BAD_CAST "level", BAD_CAST "0");
//    xmlNewProp(node, BAD_CAST "type", BAD_CAST "eval");

    node = xmlNewChild(root_node, NULL, BAD_CAST "stack_level", NULL);
    xmlNewProp(node, BAD_CAST "level", BAD_CAST "0");
    xmlNewProp(node, BAD_CAST "type", BAD_CAST "file");
    xmlNewProp(node, BAD_CAST "filename", BAD_CAST "/home/jthijssen/saffire/hello.sf");
    xmlNewProp(node, BAD_CAST "lineno", BAD_CAST "1");

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(context_get) {
    xmlNodePtr node;
    char xmlbuf[1000];

//    int depth = 0;
    int context_id = 0;

    xmlNodePtr root_node = dbgp_xml_create_response("context_get", argc, argv);
    snprintf(xmlbuf, 999, "%d", context_id);
    xmlNewProp(root_node, BAD_CAST "context", BAD_CAST xmlbuf);

    node = xmlNewChild(root_node, NULL, BAD_CAST "property", BAD_CAST "0xdeadbeef");
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "a");
    xmlNewProp(node, BAD_CAST "fullname", BAD_CAST "self.a");
    xmlNewProp(node, BAD_CAST "classname", BAD_CAST "foobar");
    xmlNewProp(node, BAD_CAST "type", BAD_CAST "numerical");

    node = xmlNewChild(root_node, NULL, BAD_CAST "property", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "f");
    xmlNewProp(node, BAD_CAST "fullname", BAD_CAST "f");
    xmlNewProp(node, BAD_CAST "classname", BAD_CAST "numerical");
    xmlNewProp(node, BAD_CAST "type", BAD_CAST "numerical");


    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(detach) {
    di->state = DBGP_STATE_STOPPED;
    di->attached = 0;

    xmlNodePtr root_node = dbgp_xml_create_response("detach", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);

    di->state = DBGP_STATE_STOPPED;

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(run) {
    di->state = DBGP_STATE_RUNNING;

    xmlNodePtr root_node = dbgp_xml_create_response("run", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);

    return root_node;
}


/**
 *
 */
static xmlNodePtr create_breakpoint_xml(t_breakpoint *bp) {
    char xmlbuf[1000];

    xmlNodePtr breakpoint_node = xmlNewNode(NULL, BAD_CAST "breakpoint");
    xmlNewProp(breakpoint_node, BAD_CAST "id", BAD_CAST bp->id);
    snprintf(xmlbuf, 999, "%d", bp->state);
    xmlNewProp(breakpoint_node, BAD_CAST "state", BAD_CAST xmlbuf);
    xmlNewProp(breakpoint_node, BAD_CAST "filename", BAD_CAST bp->filename);
    snprintf(xmlbuf, 999, "%d", bp->lineno);
    xmlNewProp(breakpoint_node, BAD_CAST "lineno", BAD_CAST xmlbuf);
    xmlNewProp(breakpoint_node, BAD_CAST "temporary", BAD_CAST (bp->temporary ? "true" : "false"));
    xmlNewProp(breakpoint_node, BAD_CAST "function", BAD_CAST bp->function);
    xmlNewProp(breakpoint_node, BAD_CAST "exception", BAD_CAST bp->exception);
    xmlNewProp(breakpoint_node, BAD_CAST "expression", BAD_CAST bp->expression);
    snprintf(xmlbuf, 999, "%d", bp->hit_value);
    xmlNewProp(breakpoint_node, BAD_CAST "hit_value", BAD_CAST xmlbuf);
    xmlNewProp(breakpoint_node, BAD_CAST "hit_condition", BAD_CAST bp->hit_condition);
    snprintf(xmlbuf, 999, "%d", bp->hit_count);
    xmlNewProp(breakpoint_node, BAD_CAST "hit_count", BAD_CAST xmlbuf);

    xmlNodePtr node = xmlNewChild(breakpoint_node, NULL, BAD_CAST "expression", BAD_CAST bp->expression);
    xmlAddChild(breakpoint_node, node);

    return breakpoint_node;
}


/**
 *
 */
DBGP_CMD_DEF(breakpoint_get) {
    int i = dbgp_args_find("-d", argc, argv);
    char *breakpoint_id = argv[i+1];

    t_breakpoint *bp = ht_find(di->breakpoints, breakpoint_id);
    xmlNodePtr bp_node = create_breakpoint_xml(bp);

    xmlNodePtr root_node = dbgp_xml_create_response("breakpoint_get", argc, argv);
    xmlAddChild(root_node, bp_node);

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(breakpoint_set) {
    char xmlbuf[1000];

    int i;
    t_breakpoint *bp = smm_malloc(sizeof(t_breakpoint));

    // Type
    i = dbgp_args_find("-t", argc, argv);
    if (i == -1) return NULL;

    if (! strcmp(argv[i+1], "line")) {
        bp->type = DBGP_BREAKPOINT_TYPE_LINE;
    } else if (! strcmp(argv[i+1], "call")) {
        bp->type = DBGP_BREAKPOINT_TYPE_CALL;
    } else if (! strcmp(argv[i+1], "return")) {
        bp->type = DBGP_BREAKPOINT_TYPE_RETURN;
    } else if (! strcmp(argv[i+1], "exception")) {
        bp->type = DBGP_BREAKPOINT_TYPE_EXCEPTION;
    } else if (! strcmp(argv[i+1], "conditional")) {
        bp->type = DBGP_BREAKPOINT_TYPE_CONDITIONAL;
    } else if (! strcmp(argv[i+1], "watch")) {
        bp->type = DBGP_BREAKPOINT_TYPE_WATCH;
    }

    // State
    i = dbgp_args_find("-s", argc, argv);
    bp->state = (i == -1) ? 1 : atoi(argv[i+1]);

    // Filename
    i = dbgp_args_find("-f", argc, argv);
    bp->filename = (i == -1) ? NULL : smm_strdup(argv[i+1]);

    // Line number
    i = dbgp_args_find("-n", argc, argv);
    bp->lineno = (i == -1) ? 0 : atoi(argv[i+1]);

    // Function
    i = dbgp_args_find("-m", argc, argv);
    bp->function = (i == -1) ? NULL : smm_strdup(argv[i+1]);

    // Exception
    i = dbgp_args_find("-x", argc, argv);
    bp->exception = (i == -1) ? NULL : smm_strdup(argv[i+1]);

    // Hit value
    i = dbgp_args_find("-h", argc, argv);
    bp->hit_value = (i == -1) ? 0 : atoi(argv[i+1]);

    // Hit condition
    i = dbgp_args_find("-o", argc, argv);
    bp->hit_condition = (i == -1) ? NULL : smm_strdup(argv[i+1]);

    // temporary
    i = dbgp_args_find("-r", argc, argv);
    bp->temporary = (i == -1) ? 0 : atoi(argv[i+1]);

    // expression
    i = dbgp_args_find("--", argc, argv);
    bp->expression = (i == -1) ? NULL : smm_strdup(argv[i+1]);

    // String representation of the breakpoint id
    di->breakpoint_id++;
    char buf[10];
    snprintf(buf, 9, "%d", di->breakpoint_id);
    bp->id = smm_strdup(buf);

    ht_add(di->breakpoints, bp->id, bp);

    xmlNodePtr root_node = dbgp_xml_create_response("breakpoint_set", argc, argv);
    snprintf(xmlbuf, 999, "%d", bp->state);
    xmlNewProp(root_node, BAD_CAST "state", BAD_CAST xmlbuf);
    xmlNewProp(root_node, BAD_CAST "id", BAD_CAST bp->id);

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(breakpoint_list) {
    xmlNodePtr root_node = dbgp_xml_create_response("breakpoint_list", argc, argv);

    t_hash_iter iter;
    ht_iter_init(&iter, di->breakpoints);
    while (ht_iter_valid(&iter)) {
        char *id = ht_iter_key(&iter);

        t_breakpoint *bp = ht_find(di->breakpoints, id);
        xmlNodePtr bp_node = create_breakpoint_xml(bp);

        xmlAddChild(root_node, bp_node);

        ht_iter_next(&iter);
    }

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(breakpoint_remove) {
    int i = dbgp_args_find("-d", argc, argv);
    char *breakpoint_id = argv[i+1];

    ht_remove(di->breakpoints, breakpoint_id);

    xmlNodePtr root_node = dbgp_xml_create_response("breakpoint_remove", argc, argv);

    return root_node;
}


/**
 *
 */
DBGP_CMD_DEF(breakpoint_update) {
    int i = dbgp_args_find("-d", argc, argv);
    char *breakpoint_id = argv[i+1];

    t_breakpoint *bp = ht_find(di->breakpoints, breakpoint_id);

    // State
    i = dbgp_args_find("-s", argc, argv);
    bp->state = (i == -1) ? 1 : atoi(argv[i+1]);

    // Line number
    i = dbgp_args_find("-n", argc, argv);
    bp->state = (i == -1) ? 0 : atoi(argv[i+1]);

    // Hit value
    i = dbgp_args_find("-h", argc, argv);
    bp->hit_value = (i == -1) ? 0 : atoi(argv[i+1]);

    // Hit condition
    i = dbgp_args_find("-o", argc, argv);
    bp->hit_condition = (i == -1) ? NULL : smm_strdup(argv[i+1]);


    xmlNodePtr root_node = dbgp_xml_create_response("breakpoint_remove", argc, argv);

    return root_node;
}
