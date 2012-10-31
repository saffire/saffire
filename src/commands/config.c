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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "command.h"
#include "config.h"
#include "general/parse_options.h"
#include "general/ini.h"
#include "general/smm.h"

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

char *ini_file = global_ini_file;


/**
 * Action: ./saffire config generate
 */
static int do_generate(void) {
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


static int ini_parse_handler(void *user, const char *section, const char *name, const char *value) {
    t_config *config = (t_config *)config;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("global", "log.level")) {
        config->global.log_level = smm_strdup(value);
    } else if (MATCH("global", "log.path")) {
        config->global.log_path = smm_strdup(value);
    } else if (MATCH("fastcgi", "pid_path")) {
        config->fastcgi.pid_path = smm_strdup(value);
    } else if (MATCH("fastcgi", "log.path")) {
        config->fastcgi.log_path = smm_strdup(value);
    } else if (MATCH("fastcgi", "daemonize")) {
        config->fastcgi.daemonize = to_bool((char *)value);
        if (config->fastcgi.daemonize == -1) config->fastcgi.daemonize = 0;
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


void read_ini(void) {
    if (ini_parse(ini_file, ini_parse_handler, &config) < 0) {
        printf("Cannot read ini settings from %s", ini_file);
        exit(1);
    }
}

/**
 * Action: ./saffire config get <setting>
 */
static int do_get(void) {
    char *setting = saffire_getopt_string(0);
}

/**
 * Action: ./saffire config set <setting> <value>
 */
static int do_set(void) {
    char *setting = saffire_getopt_string(0);
    char *value = saffire_getopt_string(1);
}

/**
 * Action: ./saffire config list <setting>
 */
static int do_list(void) {
//    read_ini();
}



static void opt_file(void *data) {
    printf("Setting ini file to :%s\n", (char *)data);
    ini_file = (char *)data;
}


static struct saffire_option global_options[] = {
        { "file", "f", required_argument, opt_file },
        { "dot", "d", required_argument, opt_file },
        { 0, 0, 0, 0 }
};

/* Config actions */
struct _argformat config_subcommands[] = {
    { "generate", "", do_generate, global_options },        // Generate new ini file
    { "get", "sslb", do_get, global_options },                 // Get a section
    { "set", "ss", do_set, global_options },                // Sets a section value
    { "list", "", do_list, global_options },                // List section value
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct _command_info info_config = {
                                        "Reads or writes configuration settings",
                                        NULL,
                                        NULL,
                                        config_subcommands,
                                        help,
                                    };
