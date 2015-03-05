/*
 Copyright (c) 2012-2015, The Saffire Group
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
#include <saffire/general/output.h>
#include <saffire/commands/command.h>
#include <saffire/general/config.h>
#include <saffire/general/smm.h>
#include <saffire/general/parse_options.h>

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
    "logo = false",
    "",
    "[debug]",
    "# Saffire only supports the dbgp protocol",
    "protocol = dbgp",
    "# Ip address to connect to, used unless remote.connect_back is enabled",
    "remote.ip = 127.0.0.1",
    "# Remote port to connect to",
    "remote.port = 9000",
    "# When true, the IP to connect back to is taken from the request (works only on fastcgi)",
    "remote.connect_back = true",
    "# Automatically start the debugger",
    "remote.autostart = true",
    "# The IDE key that is send to the IDE",
    "remote.idekey = SAFFIRE"
};


/**
 * Action: ./saffire config generate
 */
static int do_generate(void) {
    time_t current_time;
    struct tm *local_time;

    current_time = time (NULL);
    local_time = localtime (&current_time);

    output_char("#\n");
    output_char("# Default configuration file for saffire. It consists of all configuration settings.\n");
    output_char("# Generated on %s", asctime(local_time));
    output_char("#\n");

    for (int i=0; i!=ARRAY_SIZE(default_ini); i++) {
        output_char("%s\n", default_ini[i]);
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
        output_char("%s : %s\n", key, val);
        return 0;
    } else {
        output_char("Cannot find key %s\n", key);
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
 * Unsets a value from the configuration file
 *
 * Action: ./saffire config unset <setting>
 */
static int do_unset(void) {
    char *setting = saffire_getopt_string(0);

    return config_set_string(setting, NULL);
}

/**
 * Returns a list of settings that matches the search argument
 * Action: ./saffire config list <search>
 */
static int do_list(void) {
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
        char *key = ht_iter_key_str(&iter);
        char *val = ht_iter_value(&iter);
        output_char("%s : %s\n", key, val);

        ht_iter_next(&iter);
    }

    ht_destroy(matches);
    return 0;
}



/****
 * Argument Parsing and action definitions
 ***/



/* Usage string */
static const char help[]   = "Configure Saffire settings.\n"
                             "\n"
                             "Actions:\n"
                             "   generate                 Generates and outputs configuration settings\n"
                             "   get <setting>            Returns value (if set)\n"
                             "   set <setting> <value>    Set value in your configuration\n"
                             "   unset <setting>          Unsets value in your configuration\n"
                             "   list [pattern]           Returns all settings, or that matches [pattern]\n";


/* Config actions */
static struct command_action command_actions[] = {
    { "generate", "", do_generate, NULL },      // Generate new ini file
    { "get", "s", do_get, NULL },               // Get a section
    { "set", "ss", do_set, NULL },              // Sets a section value
    { "unset", "s", do_unset, NULL },           // Unsets a section value
    { "list", "|s", do_list, NULL },            // List section value (or everything)
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_config = {
    "Reads or writes configuration settings",
    command_actions,
    help,
};
