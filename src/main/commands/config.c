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
#include <string.h>
#include <time.h>
#include "general/output.h"
#include "commands/command.h"
#include "general/config.h"
#include "general/smm.h"
#include "general/parse_options.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/* Default INI settings, including comments */
static const char *default_ini[] = {
    "[global]",
    "# Global log information",
    "log.level = debug     # debug, notice, warning, error",
    "log.path = /var/log/saffire/saffire.log",
    "",
    "",
    "[gpg]",
    "# Path to the GPG application",
    "path = /usr/bin/gpg",
    "# Your GPG key to sign any modules or bytecode",
    "key = 0xFFFFFFFF",
    "",
    "",
    "[compile]",
    "# True when bytecode automatically needs to be signed",
    "sign = true",
    "",
    "",
    "[fastcgi]",
    "pid.path = /var/run/saffire.pid",
    "",
    "# Log information",
    "log.path = /var/log/saffire/fastcgi.log",
    "log.level = notice       # debug, notice, warning, error",
    "",
    "# When true, the FastCGI server will deamonize into the background",
    "daemonize = true",
    "# Number of children to spawn when daemonized",
    "spawn_children = 10",
    "",
    "# When ran as root, drop privileges to this user or uid",
    "user = -1",
    "# When ran as root, drop privileges to this group or gid",
    "group = -1",
    "",
    "# Listen to either an IP address or a Unix-socket",
    "listen = 0.0.0.0:80",
    "#listen = /tmp/saffire.socket",
    "",
    "# Number of backlogs for select(). Use -1 for default value",
    "listen.backlog = -1",
    "",
    "# Groups and permissions for Unix socket",
    "listen.socket.user = nobody",
    "listen.socket.group = nobody",
    "listen.socket.mode = 0666",
    "",
    "# FastCGI status and control URLs",
    "#status.url = /status",
    "#ping.url = /ping",
    "#ping.response = \"pong\"",
    ""
};

// Default INI file @TODO: platform specific!
char global_ini_file[] = "/etc/saffire/saffire.ini";
char user_ini_file[] = "~/saffire.ini";

char *ini_file = global_ini_file;


/**
 * Action: ./saffire config generate
 */
static int do_generate(void) {
    time_t current_time;
    struct tm *local_time;

    current_time = time (NULL);
    local_time = localtime (&current_time);

    output("#\n");
    output("# Default configuration file for saffire. It consists of all configuration settings.\n");
    output("# Generated on %s", asctime(local_time));
    output("#\n");

    for (int i=0; i!=ARRAY_SIZE(default_ini); i++) {
        output("%s\n", default_ini[i]);
    }
    return 0;
}


/**
 * Get a value from the configuration file
 *
 * Action: ./saffire config get <setting>
 */
static int do_get(void) {
    char *key = saffire_getopt_string(0);

    char *val = config_get_string(key, NULL);
    if (val) {
        output("%s : %s\n", key, val);
        return 0;
    }
    return 1;
}


/**
 * Set a value into the configuration file
 *
 * Action: ./saffire config set <setting> <value>
 */
static int do_set(void) {
    char *setting = saffire_getopt_string(0);
    char *value = saffire_getopt_string(1);

    return config_set_string(setting, value);
}


/**
 * Returns a list of settings that matches the search argument
 * Action: ./saffire config list <search>
 */
static int do_list(void) {
    char **matches;
    char *pattern = saffire_getopt_string(0);

    int count = config_get_matches(pattern, &matches);
    if (count <= 0) return 1;

    // Note: the count is always too large. The keys that do not match are NULLed out.
    for (int i=0; i!=count; i++) {
        if (matches[i] == NULL) continue;

        char *val = config_get_string(matches[i], NULL);
        output("%s : %s\n", matches[i], val);

        free(matches[i]);
    }
    free(matches);

    return 0;
}



/****
 * Argument Parsing and action definitions
 ***/


static void opt_file(void *data) {
    ini_file = (char *)data;
}

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


static struct saffire_option global_options[] = {
    { "file", "f", required_argument, opt_file },
    { "dot", "d", required_argument, opt_file },
    { 0, 0, 0, 0 }
};

/* Config actions */
static struct command_action command_actions[] = {
    { "generate", "", do_generate, global_options },        // Generate new ini file
    { "get", "s", do_get, global_options },                 // Get a section
    { "set", "ss", do_set, global_options },                // Sets a section value
    { "list", "s", do_list, global_options },               // List section value
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_config = {
    "Reads or writes configuration settings",
    command_actions,
    help,
};
