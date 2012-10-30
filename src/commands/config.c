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
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include "command.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


/* Usage string */
static const char help[]   = "Configure Saffire settings.\n"
                             "\n"
                             "Global settings:\n"
                             "    -f, --file <FILE>    File to read/write.\n"
                             "\n"
                             "Actions:\n"
                             "   generate                 Generates configuration settings\n"
                             "   get <setting>            Returns value (if set)\n"
                             "   set <setting> <value>    Set value in your configuration\n"
                             "   list                     Returns all settings\n"
                             "\n"
                             "Use * as a wildcard in a setting\n";


/* Default INI settings */
static const char *default_ini[] = {
    "[global]",
    "  # debug, notice, warning, error",
    "  log.level = debug",
    "  log.path = /var/log/saffire/saffire.log"
    "",
    "",
    "[fastcgi]",
    "  pid_path = /var/run/saffire.pid",
    "  log.path = /var/log/saffire/fastcgi.log",
    "  # debug, notice, warning, error",
    "  log.level = notice",
    "  daemonize = true",
    "",
    "  listen = 0.0.0.0:80",
    "  listen.backlog = -1",
    "  listen.socket.user = nobody",
    "  listen.socket.group = nobody",
    "  listen.socket.mode = 0666",
    "",
    "  #status.url = /status",
    "  #ping.url = /ping",
    "  #ping.response = \"pong\"",
    ""
};

// Default INI file
char *ini_file = "/etc/saffire/saffire.ini";


/**
 * Parse global options for "config" command
 */
static void do_options(int argc, char *argv[]) {
    int c;
    int option_index;

    // Suppress default errors
    opterr = 0;

    // Long options maps back to short options
    static struct option long_options[] = {
            { "file", required_argument, 0, 'f' },
            { 0, 0, 0, 0 }
    };

    // Iterate all the options
    while (1) {
        c = getopt_long (argc, argv, "f:", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 'f' :
                ini_file = argv[optind-1];
                break;
            default :
                printf("saffire: invalid option '%s'\n"
                       "Try 'saffire help config' for more information\n", argv[optind-1]);
                exit(1);
        }
    }
}

/**
 * Action: ./saffire config generate
 */
static int do_generate(int argc, char **argv) {
    time_t current_time;
    struct tm *local_time;

    current_time = time (NULL);
    local_time = localtime (&current_time);

    printf("#\n");
    printf("# Default configuration file for saffire. It consists of all configuration settings.\n");
    printf("# Generated on %s", asctime(local_time));
    printf("#\n");

    for (int i=0; i!=ARRAY_SIZE(default_ini); i++) {
        printf("%s\n", default_ini[i]);
    }
}

/**
 * Action: ./saffire config get <setting>
 */
static int do_get(int argc, char *argv[]) {
//    read_ini();
}

/**
 * Action: ./saffire config set <setting> <value>
 */
static int do_set(int argc, char *argv[]) {
}

/**
 * Action: ./saffire config list <setting>
 */
static int do_list(int argc, char *argv[]) {
//    read_ini();
}


/* Config actions */
struct _argformat config_subcommands[] = {
    { "generate", "", do_generate },        // Generate new ini file
    { "get", "s", do_get },                 // Get a section
    { "set", "ss", do_set },                // Sets a section value
    { "list", "", do_list },                // List section value
    { NULL, NULL, NULL }
};

/* Config info structure */
struct _command_info info_config = {
                                        "Reads or writes configuration settings",
                                        NULL,
                                        config_subcommands,
                                        help,
                                        do_options
                                    };
