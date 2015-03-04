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
#include <stdlib.h>
#include <fnmatch.h>
#include <libgen.h>
#ifdef __LINUX__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#include <general/config.h>
#include <general/output.h>
#include <general/smm.h>
#include <general/parse_options.h>
#include <general/path_handling.h>
#include <general/ini.h>
#include <debug.h>


#define SAFFIRE_INI_FILENAME "saffire.ini"

// Default paths
char global_ini_path[]  = "/etc/saffire";
char user_ini_path[]    = "~";
char *search_paths[] = {
    ".",                    // First find in current directory
    "~",                    // Find in user home dir
    "/etc/saffire",         // Find in specific etc directory
    "/etc",                 // Find in /etc
    NULL
};

int config_which_ini = USE_INI_SEARCH;
char *config_custom_ini_path;     // Specifies custom ini path

t_ini *config_ini = NULL;
char *config_path;      // Path
char *config_file;      // Filename

char *config_get_path(void) {
    return config_path;
}

t_ini *config_get_ini(void) {
    return config_ini;
}


char *config_seek(char *searchpaths[], char *file) {
    char path[PATH_MAX];

    int idx = 0;
    while (searchpaths[idx]) {
        snprintf(path, PATH_MAX-1, "%s/%s", searchpaths[idx], file);
        if (is_file(path)) return searchpaths[idx];

        idx++;
    }

    return NULL;
}


/*
 * Free configuration file, called by atexit()
 */
void atexit_config_free() {
    if (config_ini) {
        ini_free(config_ini);
    }
}

/**
 * Read INI file file
 */
int config_read(void) {
    atexit(atexit_config_free);

    config_file = SAFFIRE_INI_FILENAME;

    switch (config_which_ini) {
        case USE_INI_SEARCH :
        default :
            config_path = config_seek(search_paths, SAFFIRE_INI_FILENAME);
            if (config_path == NULL) return 0;   // Not found
            break;
        case USE_INI_LOCAL :
            config_path = user_ini_path;
            break;
        case USE_INI_GLOBAL :
            config_path = global_ini_path;
            break;
        case USE_INI_CUSTOM :
            config_path = config_custom_ini_path;
            if (is_file(config_path)) {
                // If we are pointing to a file, split it into dir and file
                config_file = basename(config_path);
                config_path = dirname(config_path);
            }
            break;
    }

    config_ini = ini_read(config_path, config_file);
    return (config_ini != NULL);
}


/**
 * Store value into key and save file
 */
int config_set_string(char *key, char *val) {
    if (! config_ini) return 0;

    if (! val) {
        ini_remove(config_ini, key);
    } else {
        ini_add(config_ini, key, val);
    }
    ini_save(config_ini, config_path, config_file);

    return 1;
}


/**
 * Return a string from the configuration
 */
char *config_get_string(const char *key, const char *default_value) {
    if (! config_ini) return (char *)default_value;

    char *val = ini_find(config_ini, key);
    return val ? val : (char *)default_value;
}


/**
 * Return a boolean from the configuration
 */
char config_get_bool(const char *key, char default_value) {
    if (! config_ini) return 0;

    char *val = ini_find(config_ini, key);
    if (val == NULL) return default_value;

    return to_bool(val);
}


/**
 * Return a long from the configuration
 */
long config_get_long(const char *key, long default_value) {
    if (! config_ini) return 0;

    char *val = ini_find(config_ini, key);
    if (val == NULL) return default_value;

    return atol(val);
}


/**
 * Return a list of all keys inside "matches" that matches the pattern. Returns number of matches found.
 */
t_hash_table *config_get_matches(const char *pattern, int wildcard) {
    if (! config_ini) return NULL;

    char *actual_pattern;
    smm_asprintf_char(&actual_pattern,(wildcard ? "*%s*" : "%s"), pattern);

    t_hash_table *matches = ini_match(config_ini, actual_pattern);
    smm_free(actual_pattern);

    return matches;
}
