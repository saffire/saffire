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
#ifndef __GENERAL_CONFIG_H__
#define __GENERAL_CONFIG_H__

    #include "general/hashtable.h"
    #include "general/ini.h"

    int config_which_ini;
    char *config_custom_ini_path;

    #define USE_INI_SEARCH      0       // Use search path to find ini file
    #define USE_INI_GLOBAL      1       // Use the global ini
    #define USE_INI_LOCAL       2       // Use the local user ini
    #define USE_INI_CUSTOM      3       // Use a custom ini

    int config_read(void);
    char *config_get_path(void);
    t_ini *config_get_ini(void);


    int config_set_string(char *key, char *value);
    char *config_get_string(const char *key, const char *default_value);
    char config_get_bool(const char *key, char default_value);
    long config_get_long(const char *key, long default_value);

    t_hash_table *config_get_matches(const char *pattern, int wildcard);

#endif
