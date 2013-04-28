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
//#include <sys/types.h>
//#include <arpa/inet.h>
//#include <libxml/parser.h>
//#include <libxml/tree.h>
#include <string.h>
#include "general/config.h"
#include "debugger/dbgp/xml.h"
#include "debugger/dbgp/args.h"
#include "debugger/dbgp/sock.h"




/**
 * Read actual command line
 */
static void read_commandline(int sockfd, int *argc, char ***argv) {
    char *buffer = NULL;
    int buffer_len= 0;

    for (;;) {
        char buf[128];
        bzero(buf, 128);
        int n = read(sockfd, buf, 128-1);
        buffer_len += n;
        if (! buffer) {
            buffer = (char *)malloc((buffer_len) + 1);
            buffer[0] = '\0';
        } else {
            buffer = realloc(buffer, (buffer_len) + 1);
        }

        strcat(buffer, buf);
        buffer[buffer_len] = '\0';

        char ch;
        ch = buffer[buffer_len-1];
        if (buffer[buffer_len-1] == '\0') {
            break;
        }
    }

    dbgp_args_parse(buffer, argc, argv);
}


/**
 */
xmlNodePtr do_command_context_names(int argc, char *argv[]) {
    xmlNodePtr node;

    xmlNodePtr root_node = dbgp_xml_create_response("context_names", argc, argv);

    node = xmlNewChild(root_node, NULL, BAD_CAST "context", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "Locals");
    xmlNewProp(node, BAD_CAST "id", BAD_CAST "0");

    node = xmlNewChild(root_node, NULL, BAD_CAST "context", NULL);
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "Globals");
    xmlNewProp(node, BAD_CAST "id", BAD_CAST "1");

    return root_node;
}

xmlNodePtr do_command_step_into(int argc, char *argv[]) {
    // TODO: set state DEBUGGER_RUNNING

    xmlNodePtr root_node = dbgp_xml_create_response("step_into", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST "starting");

    return root_node;
}

xmlNodePtr do_command_step_over(int argc, char *argv[]) {
    // TODO: set state DEBUGGER_RUNNING

    xmlNodePtr root_node = dbgp_xml_create_response("step_over", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST "starting");

    return root_node;
}

xmlNodePtr do_command_breakpoint_set(int argc, char *argv[]) {
    xmlNodePtr root_node = dbgp_xml_create_response("breakpoint_set", argc, argv);
    xmlNewProp(root_node, BAD_CAST "state", BAD_CAST "STATE");
    xmlNewProp(root_node, BAD_CAST "id", BAD_CAST "1");

    return root_node;
}

xmlNodePtr do_command_stack_get(int argc, char *argv[]) {
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

xmlNodePtr do_command_context_get(int argc, char *argv[]) {
    xmlNodePtr node;
    char buf[100];

//    int depth = 0;
    int context_id = 0;

    xmlNodePtr root_node = dbgp_xml_create_response("context_get", argc, argv);
    sprintf(buf, "%d", context_id);
    xmlNewProp(root_node, BAD_CAST "context", BAD_CAST buf);

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

xmlNodePtr do_command_detach(int argc, char *argv[]) {
    // TODO: set state DEBUGGER_DETACHED

    xmlNodePtr root_node = dbgp_xml_create_response("detach", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST "running");

    return root_node;
}

xmlNodePtr do_command_run(int argc, char *argv[]) {
    // TODO: set state DEBUGGER_RUNNING

    xmlNodePtr root_node = dbgp_xml_create_response("run", argc, argv);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST "running");

    return root_node;
}

struct dbgp_command {
    char *command;
    xmlNodePtr (*func)(int argc, char *argv[]);
};

struct dbgp_command dbgp_commands[] = {
    { "context_names", &do_command_context_names },
    { "context_get", &do_command_context_get },

    { "eval", &do_command_context_names },
    { "feature_set", &do_command_context_names },
    { "feature_get", &do_command_context_names },

    { "typemap_get", &do_command_context_names },
    { "property_get", &do_command_context_names },
    { "property_set", &do_command_context_names },
    { "property_value", &do_command_context_names },

    { "source", &do_command_context_names },
    { "stack_get", &do_command_stack_get },
    { "stack_depth", &do_command_stack_get },
    { "status", &do_command_stack_get },

    { "stderr", &do_command_stack_get },
    { "stdout", &do_command_stack_get },

    { "breakpoint_get", &do_command_breakpoint_set },
    { "breakpoint_set", &do_command_breakpoint_set },
    { "breakpoint_list", &do_command_breakpoint_set },
    { "breakpoint_remove", &do_command_breakpoint_set },
    { "breakpoint_update", &do_command_breakpoint_set },

    { "step_into", &do_command_step_into },
    { "step_over", &do_command_step_over },
    { "step_out", &do_command_step_over },
    { "stop", &do_command_detach },
    { "detach", &do_command_detach },
    { "run", &do_command_run },
    { NULL, NULL }
};

/**
 * Read commandline and execute the found commands
 */
static void parse_incoming_command(int sockfd) {
    char **argv;
    int argc;
    read_commandline(sockfd, &argc, &argv);

    for (int i=0; i!=argc; i++) {
        printf ("  Arg %d : '%s'\n", i, argv[i]);
    }


    struct dbgp_command *p = dbgp_commands;
    while (p->command) {
        if (! strcmp(argv[0], p->command)) {
            // Do command
            xmlNodePtr root_node = p->func(argc, argv);
            if (root_node) {
                dbgp_xml_send(sockfd, root_node);
            }
            break;
        }

        p++;
    }
    if (p->command == NULL) {
        printf("Command '%s' not found.\n", p->command);
    }

    dbgp_args_free(argv);
}



void dbgp_init(void) {
    int sockfd;
    do {
        sockfd = dbgp_sock_init();
        if (sockfd == -1) {
            printf("Can't connect to listening socket. Are you sure that the IDE is listening for debug connections?\n");
            sleep(1);
        }
    } while (sockfd == -1);

    dbgp_xml_init();

    // Send out init
    xmlNodePtr init_node = dbgp_xml_create_init_node();
    dbgp_xml_send(sockfd, init_node);

    // Parse commands and do stuff
    for (;;) {
        parse_incoming_command(sockfd);
    }

    // End
    dbgp_xml_fini();
    dbgp_sock_fini(sockfd);

//    char *debugger_host = config_get_string("debugger.host", "127.0.0.1");
//    long debugger_port = config_get_long("debugger.port", "9000");
}

void dbgp_fini(void) {
}
