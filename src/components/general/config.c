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
#include <string.h>
#include <stdlib.h>
#include <augeas.h>

#include "general/smm.h"
#include "general/parse_options.h"

const char ini_filename[] = "/etc/saffire/saffire.ini";

augeas *aug;
char prefix[] = "/files/etc/saffire/saffire.ini/";


/**
 * Convert key into a augeas-key
 */
static char *config_generate_augeas_key(const char *key) {
    char *fqn_key = smm_malloc(strlen((char *)key) + strlen(prefix));
    strcpy(fqn_key, prefix);
    strcat(fqn_key, key);

    return fqn_key;

}


/**
 * Read INI file file
 */
static void read_ini(void) {
    // Already initialized
    if (aug) return;

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
        printf("Error while saving setting: %d\n", aug_error(aug));
    }
    return 1;

}


/**
 * Return a string from the configuration
 */
char *config_get_string(const char *key) {
    read_ini();

    char *fqn_key = config_generate_augeas_key(key);
    const char *val;
    int ret = aug_get(aug, fqn_key, &val);
    smm_free(fqn_key);

    return (ret == 1) ? (char *)val : NULL;
}


/**
 * Return a boolean from the configuration
 */
char config_get_bool(const char *key) {
    read_ini();
    char *val = config_get_string(key);
    return to_bool(val);
}


/**
 * Return a long from the configuration
 */
long config_get_long(const char *key) {
    read_ini();
    char *val = config_get_string(key);
    return atol(val);
}


/**
 * Return a list of all keys inside "matches" that matches the pattern. Returns number of matches found.
 */
int config_get_matches(const char *pattern, char ***matches) {
    char **tmp_matches;
    read_ini();

    char *fqn_pattern = config_generate_augeas_key(pattern);
    int ret = aug_match(aug, fqn_pattern, &tmp_matches);
    smm_free(fqn_pattern);

    // Our matches keys are fully qualified. Remove this qualification so we can feed them
    // directly into the config_get_*() functions.
    for (int i=0; i!=ret; i++) {
        char *ptr = tmp_matches[i];
        if (strstr(ptr, prefix) == ptr) {
            memmove(ptr, ptr + strlen(prefix), strlen(ptr) - strlen(prefix)+1);
        }
    }

    *matches = tmp_matches;
    return ret;
}
