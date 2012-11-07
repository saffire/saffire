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
#include <fnmatch.h>
#include "commands/command.h"
#include "commands/config.h"
#include "general/parse_options.h"
#include "general/ini.h"
#include "general/smm.h"
#include "general/hashtable.h"
#include "general/dll.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/* Default INI settings, incuding comments */
static const char *default_ini[] = {
    "[global]",
    "  # debug, notice, warning, error",
    "  log.level = debug",
    "  log.path = /var/log/saffire/saffire.log"
    "",
    "",
    "[fastcgi]",
    "  pid.path = /var/run/saffire.pid",
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

// Default INI file @TODO: platform specific!
char global_ini_file[] = "/etc/saffire/saffire.ini";
char user_ini_file[] = "~/saffire.ini";

char *ini_file = global_ini_file;


static t_hash_table *config = NULL;                // Global configuration settings
static t_dll *dll_config = NULL;                   // Same config, but in DLL form
static int ini_read = 0;                           // 0 when ini is not yet read, 1 otherwise


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
    return 0;
}


/**
 * Handler that is called when parsing an INI line.
 */
static int ini_parse_handler(void *user, const char *section, const char *name, const char *value) {
    // Combine the section and the name
    char key[255+1];
    snprintf(key, 255, "%s.%s", section, name);

    // Add to both the hash and the DLL
    ht_add(config, key, smm_strdup(value));
    dll_append(dll_config, smm_strdup(key));

    // Return 1 for ok
    return 1;
}


/**
 * Free memory from config
 */
static void read_ini_fini(void) {
    if (config) ht_destroy(config);
    if (dll_config) dll_free(dll_config);

    config = NULL;
    dll_config = NULL;
}


/**
 * Read INI file into our hash and dll. Will return immediately when already processed
 */
static void read_ini(void) {
    // Already processed
    if (ini_read) return;

    // Register shutdown function
    atexit(read_ini_fini);

    // Create configuration hash
    config = ht_create();
    dll_config = dll_init();

    // Parse ini into hash
    if (ini_parse(ini_file, ini_parse_handler, NULL) < 0) {
        printf("Cannot read ini settings from %s", ini_file);
        exit(1);
    }

    // ini has been read
    ini_read = 1;
}


/**
 * Return a string from the configuration
 */
char *config_get_string(const char *key) {
    read_ini();

    char *val = ht_find(config, (char *)key);
    return val;
}

/**
 * Return a boolean from the configuration
 */
char config_get_bool(const char *key) {
    read_ini();

    char *val = ht_find(config, (char *)key);
    if (val == NULL) return 0;

    return to_bool(val);
}

/**
 * Return a long from the configuration
 */
long config_get_long(const char *key) {
    read_ini();

    char *val = ht_find(config, (char *)key);
    if (val == NULL) return 0;

    return atol(val);
}



/**
 * Get a value from the configuration file
 *
 * Action: ./saffire config get <setting>
 */
static int do_get(void) {
    char *key = saffire_getopt_string(0);

    char *val = config_get_string(key);
    if (val) {
        printf("%s\n", val);
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
//    char *setting = saffire_getopt_string(0);
//    char *value = saffire_getopt_string(1);

    // @TODO: Set the value inside the ini file (augeas??)
    return 1;
}

/**
 * Returns a list of settings that matches the search argument
 * Action: ./saffire config list <search>
 */
static int do_list(void) {
    int ret = 1;
    char *searchkey = saffire_getopt_string(0);
    read_ini();

    // We use the DLL here, since we cannot iterate a hash
    t_dll_element *e = DLL_HEAD(dll_config);
    while (e) {
        char *key = (char *)e->data;
        // Yeah, fnmatch(),.. so sue me...
        if (! fnmatch(searchkey, key, FNM_NOESCAPE)) {
            printf("%s = %s\n", key, config_get_string(key));
            ret = 0;
        }
        e = DLL_NEXT(e);
    }

    // Return 0 when at least 1 item is shown, 1 otherwise
    return ret;
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
