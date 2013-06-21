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
#include "debugger/dbgp/dbgp.h"
#include "debugger/dbgp/commands.h"
#include "general/output.h"
#include "debug.h"
#include "general/smm.h"

extern struct dbgp_command dbgp_commands[];
extern char *dbgp_status_names[5];


/**
 * Read actual command line
 */
static void dbgp_read_commandline(int sockfd, int *argc, char ***argv) {
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
 * Read commandline and execute the found commands
 */
static void dbgp_parse_incoming_command(t_debuginfo *di, int argc, char *argv[]) {
    if (argc == 0) return;

    for (int i=0; i!=argc; i++) {
        printf ("'%s' ", argv[i]);
    }
    printf("\n");

    struct dbgp_command *p = dbgp_commands;
    while (p->command) {
        if (! strcmp(argv[0], p->command)) {
            di->cur_cmd = p->command;

            // Save some info like current command and transaction id
            if (di->cur_txid) free(di->cur_txid);
            int i = dbgp_args_find("-i", argc, argv);
            di->cur_txid = strdup(argv[i+1]);

            // Do command
            xmlNodePtr root_node = p->func(di, argc, argv);

            // Send back XML (if any)
            if (root_node) {
                dbgp_xml_send(di->sock_fd, root_node);
            }
            break;
        }

        p++;
    }
    if (p->command == NULL) {
        xmlNodePtr root_node = dbgp_xml_create_response(di);
        xmlNodePtr node = xmlNewChild(root_node, NULL, BAD_CAST "error", NULL);
        xmlNewProp(node, BAD_CAST "code", BAD_CAST "4");
        xmlNodePtr node2 = xmlNewChild(node, NULL, BAD_CAST "error", NULL);
        xmlNodeSetContent(node2, BAD_CAST "Unimplemented command");
        dbgp_xml_send(di->sock_fd, root_node);
    }
}


/**
 *
 */
t_debuginfo *dbgp_init(t_vm_frame *frame) {
    DEBUG_PRINT(ANSI_BRIGHTBLUE "Initializing debugger" ANSI_RESET "\n");

    t_debuginfo *di = (t_debuginfo *)malloc(sizeof(t_debuginfo));

    //  Initialize structure
    di->sock_fd = 0;
    di->attached = 1;
    di->step_into = 0;
    di->step_over = 0;
    di->step_out = 0;
    di->state = DBGP_STATE_STARTING;
    di->reason = DBGP_REASON_OK;
    di->breakpoint_id = 1000;
    di->breakpoints = ht_create();
    di->cur_cmd = NULL;
    di->cur_txid = NULL;

    di->frame = frame;


    // Create a connection to the IDE.
    // @TODO: We should have a --wait flag, so when there is no connection, at least it will wait and retry every
    // second to create a connection. Up to a maximum of 100 times or so?
    do {
        di->sock_fd = dbgp_sock_init();
        if (di->sock_fd == -1) {
            printf("Can't connect to listening socket. Are you sure that the IDE is listening for debug connections?\n");
            sleep(1);
        }
    } while (di->sock_fd == -1);

    dbgp_xml_init();

    // Send out init packet
    xmlNodePtr init_node = dbgp_xml_create_init_node();
    dbgp_xml_send(di->sock_fd, init_node);

    // Do data transfers and such
    dbgp_parse_incoming_commands(di);

    printf("\n\n\n**** END OF INIT ****\n\n\n");

    return di;
}


/**
 *
 */
void dbgp_fini(t_debuginfo *di, t_vm_frame *frame) {
    di->frame = frame;

    di->state = DBGP_STATE_STOPPING;
    di->reason = DBGP_REASON_OK;

    // Set out response XML, and do final cleanup
    xmlNodePtr root_node = dbgp_xml_create_response(di);
    xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);
    xmlNewProp(root_node, BAD_CAST "reason", BAD_CAST di->reason);
    dbgp_xml_send(di->sock_fd, root_node);
    dbgp_parse_incoming_commands(di);

    // Destroy the rest
    ht_destroy(di->breakpoints);
    dbgp_xml_fini();
    dbgp_sock_fini(di->sock_fd);
}



/**
 *
 */
void dbgp_parse_incoming_commands(t_debuginfo *di) {
    char **argv;
    int argc;

    printf("dbgp_parse_incoming_commands() init\n");

    printf("Frame: %s (%d)\n", (di && di->frame && di->frame->bytecode) ? di->frame->bytecode->source_filename : "<none>", di->frame->lineno_current_line);

    // Repeat fetching commands until certain states
    while (di->state != DBGP_STATE_RUNNING && di->state != DBGP_STATE_STOPPED && di->state != DBGP_STATE_STOPPING) {
//        printf("\n *** current DI-State: %s (%d)\n", dbgp_status_names[di->state], di->state);
        dbgp_read_commandline(di->sock_fd, &argc, &argv);
        dbgp_parse_incoming_command(di, argc, argv);
        dbgp_args_free(argv);
    }

    printf("dbgp_parse_incoming_commands() fini\n");
}


void dbgp_debug(t_debuginfo *di, t_vm_frame *frame) {
    // Set the current frame in our DI object. This allows our commands to deal with frame related data
    di->frame = frame;

    printf("dbgp_debug\n");
    printf("Frame: %s (%d)\n", (di && di->frame && di->frame->bytecode) ? di->frame->bytecode->source_filename : "<none>", di->frame->lineno_current_line);

//    printf("Debug breakpoints\n");
//    printf("-----------------\n");


    /*
        STEP INTO the next statement
            * break if we are on a next line
            * break if we are on a new frame
            * break when we just started
    */

    // Check for step into
    if (di->step_into) {
        int breaking = 0;

        // Break when we just started
        if (frame->bytecode == NULL) breaking = 1;
        // Break when no frame is known
        else if (di->step_data.frame == NULL) breaking = 1;
        // Break when we are in the same frame, but different line number
        else if (di->step_data.frame == frame && di->step_data.lineno != frame->lineno_current_line) breaking = 1;
        // Break when we are inside a child frame
        else if (di->step_data.frame != frame) breaking = 1;
//        else if (di->step_data.frame == frame->parent) breaking = 1;
//        else if (di->step_data.frame->parent == frame) breaking = 1;

        if (breaking) {
            di->step_into = 0;  // Clear stepping into

            // Debugger is doing a break
            di->state = DBGP_STATE_BREAK;

            // Assume everything is ok
            di->reason = DBGP_REASON_OK;

            // Set out response XML
            xmlNodePtr root_node = dbgp_xml_create_response(di);
            xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);
            xmlNewProp(root_node, BAD_CAST "reason", BAD_CAST di->reason);
            dbgp_xml_send(di->sock_fd, root_node);

            // Parse commands from IDE
            dbgp_parse_incoming_commands(di);
            return;
        }
    }

    printf("Frame sourcefile: '%s'\n", frame->bytecode->source_filename);
    printf("LOC.FILE: '%s'\n", di->step_data.file);


    /*
        STEP OVER the next statement
            * break if we are on a next line in same frame
    */

    // Check for step_over and make sure line number has changed and we're in the current frame
    if (di->step_over) {
        int breaking = 0;

        // Break when we are not on the same line number, but in the same frame
        if (di->step_data.frame == frame && di->step_data.lineno != frame->lineno_current_line) {
            breaking = 1;
        }

        if (breaking) {
            di->step_over = 0;  // Clear stepping over

            // Debugger is doing a break
            di->state = DBGP_STATE_BREAK;

            // Assume everything is ok
            di->reason = DBGP_REASON_OK;

            // Set out response XML
            xmlNodePtr root_node = dbgp_xml_create_response(di);
            xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);
            xmlNewProp(root_node, BAD_CAST "reason", BAD_CAST di->reason);
            dbgp_xml_send(di->sock_fd, root_node);

            // Parse commands from IDE
            dbgp_parse_incoming_commands(di);
            return;
        }
    }


    /*
        STEP OUT the next statement
            * break if we hit the parent frame
    */
    if (di->step_out) {
        int breaking = 0;

        // Break until we hit the parent frame
        if (frame == di->step_data.frame) breaking = 1;

        if (breaking) {
            di->step_out = 0;  // Clear stepping out

            // Debugger is doing a break
            di->state = DBGP_STATE_BREAK;

            // Assume everything is ok
            di->reason = DBGP_REASON_OK;

            // Set out response XML
            xmlNodePtr root_node = dbgp_xml_create_response(di);
            xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);
            xmlNewProp(root_node, BAD_CAST "reason", BAD_CAST di->reason);
            dbgp_xml_send(di->sock_fd, root_node);

            // Parse commands from IDE
            dbgp_parse_incoming_commands(di);
            return;
        }
    }



    // Check for breakpoints
    t_hash_iter iter;
    for (ht_iter_init(&iter, di->breakpoints); ht_iter_valid(&iter); ht_iter_next(&iter)) {
        // Fetch breakpoint
        char *id = ht_iter_key(&iter);
        t_breakpoint *bp = ht_find(di->breakpoints, id);

        printf("%s %d %s(%d)\n", bp->id, bp->state, bp->filename, bp->lineno);

        // Skip if not enabled
        if (bp->state == 0) continue;

        if (strcmp(bp->type, DBGP_BREAKPOINT_TYPE_LINE) == 0) {
            // Check line number first, easier match
            if (bp->lineno == frame->lineno_current_line &&
                strcmp(bp->filename+7, frame->bytecode->source_filename) == 0 &&
                strncmp(bp->filename, "file://", 7) == 0) {

                // Matched line breakpoint!

                di->state = DBGP_STATE_BREAK;
                di->reason = DBGP_REASON_OK;

                xmlNodePtr root_node = dbgp_xml_create_response(di);
                xmlNewProp(root_node, BAD_CAST "status", BAD_CAST dbgp_status_names[di->state]);
                xmlNewProp(root_node, BAD_CAST "reason", BAD_CAST di->reason);
                dbgp_xml_send(di->sock_fd, root_node);

                dbgp_parse_incoming_commands(di);
            }
        }
    }
}
