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
#include "general/output.h"
#include "modules/module_api.h"
#include "objects/object.h"
#include "objects/objects.h"
#include "general/dll.h"
#include "general/smm.h"
#include "vm/vm.h"
#include "debug.h"

/**
 *
 */
SAFFIRE_MODULE_METHOD(io, print) {
    t_object *obj;

    t_dll_element *e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    while (e) {
        obj = (t_object *)e->data;

        // Implied conversion to string
        if (! OBJECT_IS_STRING(obj)) {
            t_object *string_method = object_find_attribute(obj, "__string");
            obj = vm_object_call(obj, string_method, 0);
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
    t_object *obj;

    t_dll_element *e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    obj = (t_object *)e->data;
    if (! OBJECT_IS_STRING(obj)) {
        t_object *string_method = object_find_attribute(obj, "__string");
        obj = vm_object_call(obj, string_method, 0);
    }

    char *format = ((t_string_object *)obj)->value;

    // @TODO: Don't change the args DLL!
    e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    dll_remove(SAFFIRE_METHOD_ARGS, e);

#ifdef __DEBUG
    output(ANSI_BRIGHTRED);
#endif
    output_printf(format, SAFFIRE_METHOD_ARGS);
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
    output(ANSI_BRIGHTRED "console.print: %ld arguments" ANSI_RESET "\n", SAFFIRE_METHOD_ARGS->size);
    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, printf) {
    output(ANSI_BRIGHTRED "console.printf: %ld arguments" ANSI_RESET "\n", SAFFIRE_METHOD_ARGS->size);
    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, sprintf) {
    RETURN_STRING("console.sprintf\n");
}



t_object io_struct       = { OBJECT_HEAD_INIT("io", objectTypeAny, OBJECT_TYPE_CLASS, NULL) };
t_object console_struct  = { OBJECT_HEAD_INIT("console", objectTypeAny, OBJECT_TYPE_CLASS, NULL) };


static void _init(void) {
    io_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&io_struct, "print",     CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_print);
    object_add_internal_method((t_object *)&io_struct, "printf",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_printf);
    object_add_internal_method((t_object *)&io_struct, "sprintf",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_sprintf);

    console_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&console_struct, "print",    CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_print);
    object_add_internal_method((t_object *)&console_struct, "printf",   CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_printf);
    object_add_internal_method((t_object *)&console_struct, "sprintf",  CALLABLE_FLAG_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_sprintf);
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
