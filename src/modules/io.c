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
#include <wchar.h>
#include "modules/module_api.h"
#include "interpreter/errors.h"
#include "object/object.h"
#include "object/method.h"
#include "object/string.h"
#include "general/dll.h"
#include "general/smm.h"

extern char *wctou8(const wchar_t *wstr, long len);

#ifdef __DEBUG
#define ANSI_BRIGHTRED "\33[41;33;1m"
#define ANSI_RESET "\33[0m"
#else
#define ANSI_BRIGHTRED
#define ANSI_RESET
#endif
/**
 *
 */
static t_object *io_print(t_object *self, t_dll *dll) {
    t_object *obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &obj)) {
        saffire_error("Error while parsing argument list\n");
        RETURN_SELF;
    }

    // Implied conversion to string
    if (! OBJECT_IS_STRING(obj)) {
        obj = object_call(obj, "string", 0);
    }

    char *str = wctou8(((t_string_object *)obj)->value, ((t_string_object *)obj)->char_length);
    printf(ANSI_BRIGHTRED "%s" ANSI_RESET "\n", str);
    smm_free(str);

    RETURN_SELF;
}

/**
 *
 */
static t_object *io_printf(t_object *self, t_dll *args) {
    printf(ANSI_BRIGHTRED "IO.printf: %ld arguments" ANSI_RESET "\n", args->size);
    RETURN_SELF;
}

/**
 *
 */
static t_object *io_sprintf(t_object *self, t_dll *args) {
    wchar_t tmp[] = L"IO.sprintf\n";
    RETURN_STRING(tmp);
}

/**
 *
 */
static t_object *console_print(t_object *self, t_dll *args) {
    printf(ANSI_BRIGHTRED "console.print: %ld arguments" ANSI_RESET "\n", args->size);
    RETURN_SELF;
}

/**
 *
 */
static t_object *console_printf(t_object *self, t_dll *args) {
    printf(ANSI_BRIGHTRED "console.printf: %ld arguments" ANSI_RESET "\n", args->size);
    RETURN_SELF;
}

/**
 *
 */
static t_object *console_sprintf(t_object *self, t_dll *args) {
    wchar_t tmp[] = L"console.sprintf\n";
    RETURN_STRING(tmp);
}



t_object io_struct       = { OBJECT_HEAD_INIT2("io", objectTypeCustom, NULL, NULL, OBJECT_TYPE_CLASS, NULL) };
t_object console_struct  = { OBJECT_HEAD_INIT2("console", objectTypeCustom, NULL, NULL, OBJECT_TYPE_CLASS, NULL) };


static void _init(void) {
    io_struct.methods = ht_create();
    object_add_internal_method(&io_struct, "printf", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, io_print);
    object_add_internal_method(&io_struct, "print", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, io_print);
    object_add_internal_method(&io_struct, "printf", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, io_printf);
    object_add_internal_method(&io_struct, "sprintf", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, io_sprintf);
    io_struct.properties = ht_create();

    console_struct.methods = ht_create();
    object_add_internal_method(&console_struct, "print", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, console_print);
    object_add_internal_method(&console_struct, "printf", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, console_printf);
    object_add_internal_method(&console_struct, "sprintf", METHOD_FLAG_STATIC, METHOD_VISIBILITY_PUBLIC, console_sprintf);
    console_struct.properties = ht_create();
}

static void _fini(void) {
    // Destroy methods and properties
    ht_destroy(io_struct.methods);
    ht_destroy(io_struct.properties);

    ht_destroy(console_struct.methods);
    ht_destroy(console_struct.properties);

}


static t_object *_objects[] = {
    &io_struct,
    &console_struct,
    NULL
};

t_module module_io = {
    "::_sfl::io",
    "Standard I/O module",
    _objects,
    _init,
    _fini,
};
