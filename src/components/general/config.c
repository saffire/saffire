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
#include <stdlib.h>
#include <fnmatch.h>

#include "general/output.h"
#include "general/smm.h"
#include "general/parse_options.h"
#include "general/ini.h"

t_ini *config_ini = NULL;
char *config_path;


/**
 * Read INI file file
 */
int config_init(char *path) {
    config_path = path;
    config_ini = ini_read(config_path);
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
    ini_save(config_ini, config_path);

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
    asprintf(&actual_pattern,(wildcard ? "*%s*" : "%s"), pattern);

    t_hash_table *matches = ini_match(config_ini, actual_pattern);
    free(actual_pattern);

    return matches;
}
