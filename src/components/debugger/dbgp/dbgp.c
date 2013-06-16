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
    for (int i=0; i!=argc; i++) {
        printf ("  Arg %d : '%s'\n", i, argv[i]);
    }

    struct dbgp_command *p = dbgp_commands;
    while (p->command) {
        if (! strcmp(argv[0], p->command)) {
            // Do command
            xmlNodePtr root_node = p->func(di, argc, argv);
            if (root_node) {
                dbgp_xml_send(di->sock_fd, root_node);
            }
            break;
        }

        p++;
    }
    if (p->command == NULL) {
        printf("Command '%s' not found.\n", p->command);
    }
}


/**
 *
 */
t_debuginfo *dbgp_init(void) {
    DEBUG_PRINT(ANSI_BRIGHTBLUE "Initializing debugger" ANSI_RESET "\n");

    t_debuginfo *di = (t_debuginfo *)malloc(sizeof(t_debuginfo));

    //  Initialize structure
    di->sock_fd = 0;
    di->attached = 1;
    di->step_into = 0;
    di->state = DBGP_STATE_STARTING;
    di->breakpoint_id = 1000;
        di->breakpoints = ht_create();


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

    return di;
}


/**
 *
 */
void dbgp_fini(t_debuginfo *di) {
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

    printf("\n\n\nPARSING!\n\n\n\n\n\n");

    // Repeat fetching commands until our certain status

    while (di->state != DBGP_STATE_RUNNING && di->state != DBGP_STATE_STOPPED) {
        printf("current DI-State: %s (%d)\n", dbgp_status_names[di->state], di->state);
        dbgp_read_commandline(di->sock_fd, &argc, &argv);
        dbgp_parse_incoming_command(di, argc, argv);
        dbgp_args_free(argv);
    }

    printf("\n\n\nDONE PARSING!\n\n\n\n\n\n");
}
