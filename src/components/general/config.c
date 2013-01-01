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
#include <string.h>
#include <stdlib.h>
#include <augeas.h>
#include <fnmatch.h>

#include "general/output.h"
#include "general/smm.h"
#include "general/parse_options.h"

const char ini_filename[] = "/etc/saffire/saffire.ini";

augeas *aug;
char prefix[] = "/files/etc/saffire/saffire.ini/";
char allmatch[] = "*/*[label() != '#comment']";

/**
 * Convert key into a augeas-key
 */
static char *config_generate_augeas_key(const char *key) {
    char *tmp_key = smm_strdup(key);

    // Make sure the first dot is changed to /
    char *p = strchr(tmp_key, '.');
    if (p != NULL) {
        *p = '/';
    }


    char *fqn_key = smm_malloc(strlen((char *)tmp_key) + strlen(prefix) + 1);
    strcpy(fqn_key, prefix);
    strcat(fqn_key, tmp_key);

    smm_free(tmp_key);
    return fqn_key;

}

void read_ini_fini(void) {
    if (aug) aug_close(aug);
}

/**
 * Read INI file file
 */
static void read_ini(void) {
    // Already initialized
    if (aug) return;

    atexit(read_ini_fini);

    // Initialize augeas
    aug = aug_init("/", "/usr/share/augeas/lenses/dist", AUG_NO_STDINC | AUG_NO_MODL_AUTOLOAD);
    aug_set(aug, "/augeas/load/iniparse/lens", "Puppet.lns");
    aug_set(aug, "/augeas/load/iniparse/incl", ini_filename);
    aug_load(aug);
}


/**
 * Store value into key and save file
 */
int config_set_string(char *key, char *val) {
    read_ini();

    // Store fully qualified keyname with value
    char *fqn_key = config_generate_augeas_key(key);
    aug_set(aug, fqn_key, val);
    smm_free(fqn_key);

    // Save
    int ret = aug_save(aug);
    if (ret == -1) {
        error("Error while saving setting: %d\n", aug_error(aug));
    }
    return 1;

}


/**
 * Return a string from the configuration
 */
char *config_get_string(const char *key, const char *default_value) {
    read_ini();

    char *fqn_key = config_generate_augeas_key(key);
    const char *val;
    int ret = aug_get(aug, fqn_key, &val);
    smm_free(fqn_key);

    return (ret == 1) ? (char *)val : (char *)default_value;
}


/**
 * Return a boolean from the configuration
 */
char config_get_bool(const char *key, char default_value) {
    read_ini();
    char *val = config_get_string(key, NULL);
    if (val == NULL) return default_value;

    return to_bool(val);
}


/**
 * Return a long from the configuration
 */
long config_get_long(const char *key, long default_value) {
    read_ini();
    char *val = config_get_string(key, NULL);
    if (val == NULL) return default_value;

    return atol(val);
}


/**
 * Return a list of all keys inside "matches" that matches the pattern. Returns number of matches found.
 */
int config_get_matches(const char *pattern, char ***matches) {
    char **tmp_matches;
    read_ini();

    // Create a fully qualified augeas name (/files/...)
    char *fqn_pattern = config_generate_augeas_key(allmatch);
    int ret = aug_match(aug, fqn_pattern, &tmp_matches);
    smm_free(fqn_pattern);

    // Our matches keys are fully qualified. Remove this qualification so we can feed them
    // directly into the config_get_*() functions.
    for (int i=0; i!=ret; i++) {
        char *ptr = tmp_matches[i];
        // Remove the "augeas" part of the matched key
        if (strstr(ptr, prefix) == ptr) {
            memmove(ptr, ptr + strlen(prefix), strlen(ptr) - strlen(prefix)+1);
        }

        // change section/name to section.name
        char *ptr2 = strchr(ptr, '/');
        if (ptr2) *ptr2 = '.';

        // pattern becomes *pattern*
        char *wc_pattern = smm_malloc(strlen(pattern) + 3);
        bzero(wc_pattern, strlen(pattern) + 3);
        strcpy(wc_pattern, "*");
        strcat(wc_pattern, pattern);
        strcat(wc_pattern, "*");

        // Check if it matches
        if (fnmatch(wc_pattern, ptr, 0) != 0) {
            free(ptr);
            tmp_matches[i] = NULL;  // Just clear this entry.. not in use
        }
    }

    *matches = tmp_matches;
    return ret;
}
