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
    "log.path = /var/log/saffire/saffire.log",
    "# Values for log.level: debug, notice, warning, error",
    "log.level = debug",
    "",
    "",
    "[gpg]",
    "# Path to the GPG application",
    "path = /usr/bin/gpg",
    "# Your GPG key to sign any modules or bytecode",
    "key = ",
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
    "# Values for log.level: debug, notice, warning, error",
    "log.level = notice",
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
    "",
    "[repl]",
    "# Default REPL command prompt. Supports the following placeholders: ",
    "#   %%   Literal %",
    "#   %a   ANSI Escape code",
    "#   %d   Date (dd/mm/yyyy)",
    "#   %D   Date (mm/dd/yyyy)",
    "#   %l   Line number",
    "#   %n   Newline",
    "#   %x   Context",
    "#   %t   time (h:i:s)",
    "#   %T   time (H:i:s)",
    "ps1 = (%a[33m;%l%a[0m;) [%a[34m;%x%a[0m;] >",
    "",
    "# Second command prompt which is used when entering more lines",
    "ps2 = ... >",
    "",
    "# Readline editor mode: emacs or vi",
    "editor = emacs",
    "",
    "# History file to save repl history",
    "history.file = ~/.saffire.history",
    "",
    "# Maximum number of items in the history file",
    "history.size = 800",
    "",
    "# Display the saffire logo upon start of the repl",
    "logo = false"
};

// Default INI file @TODO: platform specific!
char global_ini_path[]  = "/etc/saffire/saffire.ini";
char user_ini_path[]    = "~/saffire.ini";

#define USE_INI_GLOBAL      1       // Use the global ini
#define USE_INI_LOCAL       2       // Use the local user ini
#define USE_INI_CUSTOM      3       // Use a custom ini

int which_ini = USE_INI_LOCAL;
char *custom_ini_path;     // Specifies custom ini path


static void read_config(void) {
    char *ini_path;

    switch (which_ini) {
        case USE_INI_LOCAL :
        default :
            ini_path = user_ini_path;
            break;
        case USE_INI_GLOBAL :
            ini_path = global_ini_path;
            break;
        case USE_INI_CUSTOM :
            ini_path = custom_ini_path;
            break;
    }

    if (! config_init(ini_path)) {
        output("Error: cannot read the configuration file '%s'\n", ini_path);
    }
}


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
    read_config();

    char *key = saffire_getopt_string(0);

    char *val = config_get_string(key, NULL);
    if (val) {
        output("%s : %s\n", key, val);
        return 0;
    } else {
        output("Cannot find key %s\n", key);
    }
    return 1;
}


/**
 * Set a value into the configuration file
 *
 * Action: ./saffire config set <setting> <value>
 */
static int do_set(void) {
    read_config();

    char *setting = saffire_getopt_string(0);
    char *value = saffire_getopt_string(1);

    return config_set_string(setting, value);
}


/**
 * Unsets a value from the configuration file
 *
 * Action: ./saffire config unset <setting>
 */
static int do_unset(void) {
    read_config();

    char *setting = saffire_getopt_string(0);

    return config_set_string(setting, NULL);
}

/**
 * Returns a list of settings that matches the search argument
 * Action: ./saffire config list <search>
 */
static int do_list(void) {
    read_config();

    char *pattern = saffire_getopt_string(0);
    if (! pattern) pattern = "";

    // Find matches
    t_hash_table *matches = config_get_matches(pattern, 1);
    if (! matches || matches->element_count == 0) {
        ht_destroy(matches);
        return 1;
    }

    // At least one match found, iterate them
    t_hash_iter iter;
    ht_iter_init(&iter, matches);
    while (ht_iter_valid(&iter)) {
        char *key = ht_iter_key(&iter);
        char *val = ht_iter_value(&iter);
        output("%s : %s\n", key, val);

        ht_iter_next(&iter);
    }

    ht_destroy(matches);
    return 0;
}



/****
 * Argument Parsing and action definitions
 ***/


static void opt_file(void *data) {
    which_ini = USE_INI_CUSTOM;
    custom_ini_path = (char *)data;
}

static void opt_global(void *data) {
    which_ini = USE_INI_GLOBAL;
}

/* Usage string */
static const char help[]   = "Configure Saffire settings.\n"
                             "\n"
                             "Global settings:\n"
                             "    -f, --file <filename>   File to read/write.\n"
                             "    --global                Write to global configuration file.\n"
                             "\n"
                             "Actions:\n"
                             "   generate                 Generates configuration settings\n"
                             "   get <setting>            Returns value (if set)\n"
                             "   set <setting> <value>    Set value in your configuration\n"
                             "   unset <setting>          Unsets value in your configuration\n"
                             "   list [pattern]           Returns all settings, or that matches [pattern]\n"
                             "\n"
                             "Configuration paths:\n"
                             "  By default, saffire will use ~/.saffire.ini, when the --global has been \n"
                             "  added as an option, it will try to read/write to /etc/saffire/saffire.ini\n"
                             "  if the -f or --file has been entered, it will use the given path \n";


static struct saffire_option global_options[] = {
    { "file", "f", required_argument, opt_file },
    { "global", "", no_argument, opt_global },
    { 0, 0, 0, 0 }
};

/* Config actions */
static struct command_action command_actions[] = {
    { "generate", "", do_generate, NULL },        // Generate new ini file
    { "get", "s", do_get, global_options },       // Get a section
    { "set", "ss", do_set, global_options },      // Sets a section value
    { "unset", "s", do_unset, global_options },   // Unsets a section value
    { "list", "|s", do_list, global_options },    // List section value (or everything)
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_config = {
    "Reads or writes configuration settings",
    command_actions,
    help,
};
