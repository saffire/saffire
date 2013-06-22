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
#include <pcre.h>
#include <fnmatch.h>
#include <ctype.h>
#include "general/ini.h"
#include "general/smm.h"
#include "general/dll.h"


/**
 * Parses a ini-file into keys and section_endings.
 */
static void ini_parse(t_ini *ini) {
    int offsets[30];
    pcre *section_re;
    pcre *key_re;
    const char *error;
    int error_offset;
    int rc;
    char *match1;
    char *match2;


    // We default to the global section
    char *section = smm_strdup("global");

    // Sanity check
    if (! ini || ! ini->_private.ini_lines) return;

    // Destroy section endings, we allocate later on
    if (ini->_private.section_endings) {
        ht_destroy(ini->_private.section_endings);
        ini->_private.section_endings = ht_create();
    }

    // Destroy our keys, we allocate later on
    if (ini->keys) {
        ht_destroy(ini->keys);
        ini->keys = ht_create();
    }

    // Precompile our regexes we will be using
    section_re = pcre_compile("\\[([^\\]]+)\\]", 0, &error, &error_offset, 0);
    key_re = pcre_compile("^([a-z0-9_.\\[\\]-]+)\\s*=\\s*(.*)$", PCRE_CASELESS, &error, &error_offset, 0);

    // Iterate all lines
    int lineno = 1;
    t_dll_element *e = DLL_HEAD(ini->_private.ini_lines);
    while (e) {
        rc = pcre_exec(section_re, 0, e->data, strlen(e->data), 0, 0, offsets, 30);
        if (rc >= 1) {
            // Found a [section] line
            pcre_get_substring(e->data, offsets, rc, 1, (const char **)&match1);
            smm_free(section);
            section = smm_strdup(match1);
            pcre_free_substring(match1);
        } else {
            rc = pcre_exec(key_re, 0, e->data, strlen(e->data), 0, 0, offsets, 30);
            if (rc == 3) {
                // Found a "key = val" line
                pcre_get_substring(e->data, offsets, rc, 1, (const char **)&match1);
                pcre_get_substring(e->data, offsets, rc, 2, (const char **)&match2);

                // Create full section.key
                int len = strlen(section) + strlen(match1) + 2;
                char *fullkey = smm_malloc(len);
                bzero(fullkey, len);
                strcpy(fullkey, section);
                strcat(fullkey, ".");
                strcat(fullkey, match1);

                // Create IE element and add to our key hash
                t_ini_element *ie = (t_ini_element *)smm_malloc(sizeof(t_ini_element));
                ie->offset = lineno;
                ie->value = smm_strdup(match2);
                ht_add(ini->keys, fullkey, ie);

                pcre_free_substring(match1);
                pcre_free_substring(match2);
            }
        }

        // Assume that this line is the last line of the section.
        int empty = 1;
        char *p = (char *)e->data;
        for (int i=0; i!=strlen(p); i++) {
            if (! isspace(p[i])) {
                empty = 0;
                break;
            }
        }
        if (! empty) {
            if (ht_exists(ini->_private.section_endings, section)) {
                ht_replace(ini->_private.section_endings, section, (void *)lineno);
            } else {
                ht_add(ini->_private.section_endings, section, (void *)lineno);
            }
        }

        // Advance to the next line
        lineno++;
        e = DLL_NEXT(e);
    }

    // @TODO: Free PCRE?
    smm_free(section);
}



/**
 * Read a file and returns as a (parsed) ini
 */
t_ini *ini_read(const char *filename) {
    t_ini *ini = (t_ini *)smm_malloc(sizeof(t_ini));

    ini->_private.ini_lines = dll_init();
    ini->_private.section_endings = ht_create();
    ini->keys = ht_create();

    FILE *f = fopen(filename, "r");
    if (! f) return NULL;

    char line[2048];
    while (fgets(line, 2048, f) != NULL) {
        // Trim CR+LFs
        while (line[strlen(line)-1] == '\r' ||
               line[strlen(line)-1] == '\n') {
            line[strlen(line)-1] = '\0';
        }
        dll_append(ini->_private.ini_lines, smm_strdup(line));
    }

    ini_parse(ini);
    return ini;
}

/**
 * Frees up an allocated INI structure
 */
void ini_free(t_ini *ini) {
    if (! ini) return;

    if (ini->_private.ini_lines) {
        // Free ini lines
        t_dll_element *e = DLL_HEAD(ini->_private.ini_lines);
        while (e) {
            smm_free(e->data);
            e = DLL_NEXT(e);
        }
        dll_free(ini->_private.ini_lines);
    }

    // Free section endings
    if (ini->_private.section_endings)
        ht_destroy(ini->_private.section_endings);

    // Free up actual keys
    if (ini->keys)
        ht_destroy(ini->keys);

    // And finally, free the ini itself
    smm_free(ini);
}


/**
 *
 */
t_hash_table *ini_match(t_ini *ini, const char *pattern) {
    t_hash_table *matches = ht_create();

    t_hash_iter iter;
    ht_iter_init(&iter, ini->keys);
    while (ht_iter_valid(&iter)) {
        char *key = ht_iter_key(&iter);

        if (fnmatch(pattern, key, FNM_CASEFOLD) == 0) {
            t_ini_element *ie = ht_iter_value(&iter);
            ht_add(matches, key, ie->value);
        }

        ht_iter_next(&iter);
    }

    return matches;
}


/**
 *
 */
char *ini_find(t_ini *ini, const char *key) {
    if (ht_exists(ini->keys, key)) {
        t_ini_element *ie = ht_find(ini->keys, key);
        return ie->value;
    }

    return NULL;
}


/**
 * Make sure you free the key after usage!
 */
static char *ini_get_key(const char *key) {
    char *k;

    char *dotpos = strstr(key, ".");
    if (dotpos == NULL) {
        k = smm_strdup(key);
    } else {
        int len = strlen(key) - (dotpos - key);
        k = (char *)smm_malloc(len + 1);
        bzero(k, len + 1);
        strncpy(k, dotpos+1, len);
    }
    return k;
}

static char *ini_get_section(const char *key) {
    char *section;

    char *dotpos = strstr(key, ".");
    if (!dotpos) {
        section = smm_strdup("global");
    } else {
        int len = (dotpos - key);
        section = (char *)smm_malloc(len + 1);
        bzero(section, len + 1);
        strncpy(section, key, len);
    }
    return section;
}

/**
 * Adds or replaces a key / value pair.
 *
 * Key must be fully qualified: <section>.<key>
 */
void ini_add(t_ini *ini, const char *key, const char *val) {
    int offset;
    char *tmp = ini_get_key(key);

    // Generate the complete line we need to store
    int len = strlen(tmp) + 3 + strlen(val) + 1;
    char *line = smm_malloc(len);
    bzero(line, len);

    strcpy(line, tmp);
    strcat(line, " = ");
    strcat(line, val);
    smm_free(tmp);

    if (ht_exists(ini->keys, key)) {
       t_ini_element *ie = ht_find(ini->keys, key);
       if (ie->value) smm_free(ie->value);
       ie->value = smm_strdup(val);

        t_dll_element *e = dll_seek_offset(ini->_private.ini_lines, ie->offset - 1);
        smm_free(e->data);
        e->data = smm_strdup(line);
    } else {
        char *section = ini_get_section(key);

        if (ht_exists(ini->_private.section_endings, section)) {
            // Section already exists. Find the offset of the last line in that section
            offset = (int)ht_find(ini->_private.section_endings, section);
        } else {
            // Append the new section at the end of the file
            dll_append(ini->_private.ini_lines, smm_strdup(""));

            char buf[1024];
            snprintf(buf, 1023, "[%s]", section);
            dll_append(ini->_private.ini_lines, smm_strdup(buf));

            // We can add our line to the end of the file (as it is the last section)
            offset = ini->_private.ini_lines->size;
        }

        // Add line after the 'offset'-th element
        t_dll_element *e = dll_seek_offset(ini->_private.ini_lines, offset-1);
        dll_insert_after(ini->_private.ini_lines, e, smm_strdup(line));

        smm_free(section);
    }

    smm_free(line);

    // Reparse
    ini_parse(ini);
}


/**
 * Removes a key from the ini file.
 * Returns 0 on failure, 1 on success
 */
int ini_remove(t_ini *ini, const char *key) {
    if (! ht_exists(ini->keys, key)) return 0;

    t_ini_element *ie = ht_find(ini->keys, key);
    t_dll_element *e = dll_seek_offset(ini->_private.ini_lines, ie->offset - 1);
    dll_remove(ini->_private.ini_lines, e);

    ht_remove(ini->keys, key);

    // Reparse
    ini_parse(ini);
    return 1;
}


/**
 * Saves current ini back into 'filename'
 * Returns 0 on failuire, 1 on success
 */
int ini_save(t_ini *ini, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return 0;

    t_dll_element *e = DLL_HEAD(ini->_private.ini_lines);
    while (e) {
        fprintf(f, "%s\n", (char *)e->data);
        e = DLL_NEXT(e);
    }

    return 1;
}
