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
#ifndef __INI_H__
#define __INI_H__

    #include <saffire/general/dll.h>
    #include <saffire/general/hashtable.h>

    typedef struct _ini {
        // Publicly available stuff
        t_hash_table *keys;

        // --- Not so public part
        struct _private {
            t_dll *ini_lines;
            t_hash_table *section_endings;
        } _private;
    } t_ini;

    typedef struct _ini_element {
        char *value;
        int offset;
    } t_ini_element;



    t_ini *ini_read(const char *path, char *filename);
    void ini_free(t_ini *ini);
    t_hash_table *ini_match(t_ini *ini, const char *pattern);
    char *ini_find(t_ini *ini, const char *key);
    void ini_add(t_ini *ini, const char *key, const char *val);
    int ini_remove(t_ini *ini, const char *key);
    int ini_save(t_ini *ini, const char *path, const char *filename);

#endif
