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
#include "general/output.h"
#include "modules/module_api.h"
#include "objects/object.h"
#include "objects/string.h"
#include "objects/attrib.h"
#include "objects/method.h"
#include "general/dll.h"
#include "general/smm.h"

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
SAFFIRE_MODULE_METHOD(io, print) {
    t_object *obj, *attrib;

    t_dll_element *e = DLL_HEAD(args);
    while (e) {
        obj = (t_object *)e->data;

        // Implied conversion to string
        if (! OBJECT_IS_STRING(obj)) {
            attrib = object_find_attribute(obj, "string");
            obj = object_call(obj, attrib, 0);
        }

        output(ANSI_BRIGHTRED "%s" ANSI_RESET, ((t_string_object *)obj)->value);

        e = DLL_NEXT(e);
    }

    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io, printf) {
    t_string_object *obj;

    t_dll_element *e = DLL_HEAD(args);
    obj = (t_string_object *)e->data;
    if (! OBJECT_IS_STRING(obj)) {
        error_and_die(1, "argument 1 of printf needs to be a string\n");
    }

    char *format = obj->value;

    // @TODO: Don't change the args DLL!
    e = DLL_HEAD(args);
    dll_remove(args, e);

#ifdef __DEBUG
    output(ANSI_BRIGHTRED);
#endif
    output_printf(format, args);
#ifdef __DEBUG
    output(ANSI_RESET);
#endif

    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io, sprintf) {
    RETURN_STRING("IO.sprintf\n");
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, print) {
    output(ANSI_BRIGHTRED "console.print: %ld arguments" ANSI_RESET "\n", args->size);
    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, printf) {
    output(ANSI_BRIGHTRED "console.printf: %ld arguments" ANSI_RESET "\n", args->size);
    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, sprintf) {
    RETURN_STRING("console.sprintf\n");
}



t_object io_struct       = { OBJECT_HEAD_INIT2("io", objectTypeAny, NULL, NULL, OBJECT_TYPE_CLASS, NULL) };
t_object console_struct  = { OBJECT_HEAD_INIT2("console", objectTypeAny, NULL, NULL, OBJECT_TYPE_CLASS, NULL) };


static void _init(void) {
    io_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&io_struct, "print",     METHOD_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_print);
    object_add_internal_method((t_object *)&io_struct, "printf",    METHOD_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_printf);
    object_add_internal_method((t_object *)&io_struct, "sprintf",   METHOD_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_sprintf);

    console_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&console_struct, "print",    METHOD_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_print);
    object_add_internal_method((t_object *)&console_struct, "printf",   METHOD_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_printf);
    object_add_internal_method((t_object *)&console_struct, "sprintf",  METHOD_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_sprintf);
}

static void _fini(void) {
    object_remove_all_internal_attributes(&io_struct);
    ht_destroy(io_struct.attributes);

    object_remove_all_internal_attributes(&console_struct);
    ht_destroy(console_struct.attributes);
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
