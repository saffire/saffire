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
#include <saffire/general/output.h>
#include <saffire/modules/module_api.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/dll.h>
#include <saffire/memory/smm.h>
#include <saffire/vm/vm.h>
#include <saffire/debug.h>


/**
 *
 */
static t_object *_saffire_print(t_object *self, t_dll *arguments, int newline) {
    t_object *obj;

    t_dll_element *e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    while (e) {
        obj = DLL_DATA_PTR(e);

        // Implied conversion to string
        if (! OBJECT_IS_STRING(obj)) {
            t_attrib_object *string_method = object_attrib_find(obj, "__string");
            obj = call_saffire_method(obj, string_method, 0);
        }

        if (obj) {
            output_char("%s", OBJ2STR0(obj));
        } else {
            output_char("");
        }

        e = DLL_NEXT(e);
    }

    if (newline) {
        output_char("\n");
    }
    output_flush();

    RETURN_SELF;
}

static void _io_dump(t_object *obj, int depth) {
    t_attrib_object *method;
    t_object *iter_obj;
    t_object *ret;

    char *padding_depth = smm_malloc(depth * 4);
    memset(padding_depth, ' ', depth * 4);
    padding_depth[depth * 4] = 0;

    // fetch iterator
    method = object_attrib_find(obj, "__iterator");
    iter_obj = call_saffire_method(obj, method, 0);

    // We don't know how the iterator is implemented internally, so we just call the iterator functionality through call_saffire_method

    method = object_attrib_find(iter_obj, "__rewind");
    ret = call_saffire_method(iter_obj, method, 0);
    object_release(ret);

    do {
        method = object_attrib_find(iter_obj, "__hasNext");
        ret = call_saffire_method(iter_obj, method, 0);
        if (IS_BOOLEAN_FALSE(ret)) {
            object_release(ret);
            break;
        } else {
            object_release(ret);
        }

        method = object_attrib_find(iter_obj, "__key");
        t_object *key = call_saffire_method(iter_obj, method, 0);

        method = object_attrib_find(iter_obj, "__value");
        t_object *val = call_saffire_method(iter_obj, method, 0);

        //

        t_string_object *key_str;
        t_string_object *val_str;

        if (! OBJECT_IS_STRING(key)) {
            t_attrib_object *string_method = object_attrib_find(key, "__string");
            key_str = (t_string_object *)call_saffire_method(key, string_method, 0);
            object_release((t_object *)key);
        } else {
            key_str = (t_string_object *)key;
        }

        if (object_has_interface(val, "Iterator") && ! OBJECT_IS_STRING(val)) {
            module_io_print("%s [%s] => \n", padding_depth, OBJ2STR0(key_str));

            // Level down
            _io_dump(val, depth + 1);

            // Don't display something on this level
            val_str = NULL;
        } else if (! OBJECT_IS_STRING(val)) {
            // Convert to string
            t_attrib_object *string_method = object_attrib_find(val, "__string");
            val_str = (t_string_object *)call_saffire_method(val, string_method, 0);
            object_release((t_object *)val);
        } else {
            // Otherwise, just print string
            val_str = (t_string_object *)val;
        }

        if (val_str) {
            module_io_print("%s [%s] => %s\n", padding_depth, OBJ2STR0(key_str), OBJ2STR0(val_str));
        }

        if (val_str) {
            object_release((t_object *)val_str);
        }
        object_release((t_object *)key_str);

        method = object_attrib_find(iter_obj, "__next");
        ret = call_saffire_method(iter_obj, method, 0);
        object_release(ret);
    } while (1);

    smm_free(padding_depth);
}



/**
 *
 */
SAFFIRE_MODULE_METHOD(io, print) {
    return _saffire_print(self, arguments, 0);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io, println) {
    return _saffire_print(self, arguments, 1);
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io, printf) {
    t_dll_element *e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    t_object *obj = DLL_DATA_PTR(e);
    if (! OBJECT_IS_STRING(obj)) {
        t_attrib_object *string_method = object_attrib_find(obj, "__string");
        obj = call_saffire_method(obj, string_method, 0);
    }

    t_string *format = ((t_string_object *)obj)->data.value;

    // @TODO: Don't change the args DLL!
    e = DLL_HEAD(SAFFIRE_METHOD_ARGS);
    dll_remove(SAFFIRE_METHOD_ARGS, e);

#ifdef __DEBUG
    output_char(ANSI_BRIGHTYELLOW);
#endif
    output_string_printf(format, SAFFIRE_METHOD_ARGS);
#ifdef __DEBUG
    output_char(ANSI_RESET);
#endif

    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io, sprintf) {
    RETURN_STRING_FROM_CHAR("IO.sprintf\n");
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(io, dump) {
    t_object *iter_obj;

    if (object_parse_arguments(SAFFIRE_METHOD_ARGS, "o",  &iter_obj) != 0) {
        return NULL;
    }

    if (! object_has_interface(iter_obj, "Iterator")) {
        object_raise_exception(Object_InterfaceException, 1, "io.dump() can only dump objects with an __iterable interface");
        return NULL;
    }

    _io_dump(iter_obj, 0);

    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, print) {
    output_char("console.print: %ld arguments\n", SAFFIRE_METHOD_ARGS->size);
    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, printf) {
    output_char("console.printf: %ld arguments\n", SAFFIRE_METHOD_ARGS->size);
    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(console, sprintf) {
    RETURN_STRING_FROM_CHAR("console.sprintf\n");
}



t_object io_struct       = { OBJECT_HEAD_INIT("io", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0), OBJECT_FOOTER };
t_object console_struct  = { OBJECT_HEAD_INIT("console", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0), OBJECT_FOOTER };


static void _init(void) {
    io_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&io_struct, "print",     ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_print);
    object_add_internal_method((t_object *)&io_struct, "println",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_println);
    object_add_internal_method((t_object *)&io_struct, "printf",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_printf);
    object_add_internal_method((t_object *)&io_struct, "sprintf",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_sprintf);
    object_add_internal_method((t_object *)&io_struct, "dump",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_io_method_dump);

    console_struct.attributes = ht_create();
    object_add_internal_method((t_object *)&console_struct, "print",    ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_print);
    object_add_internal_method((t_object *)&console_struct, "printf",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_printf);
    object_add_internal_method((t_object *)&console_struct, "sprintf",  ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_console_method_sprintf);
}

static void _fini(void) {
    object_free_internal_object(&io_struct);
    object_free_internal_object(&console_struct);
}


static t_object *_objects[] = {
    &io_struct,
    &console_struct,
    NULL
};

t_module module_io = {
    "\\saffire",
    "Standard I/O module",
    _objects,
    _init,
    _fini,
};



/**
 * Additional method to be called from internal modules
 *
 * @param format
 * @param ...
 * @return
 */
t_object *module_io_print(char *format, ...) {
    t_dll *dll_args = dll_init();

    va_list args;
    va_start(args, format);

    char *s = NULL;
    if (args == NULL) {
        smm_asprintf_char(&s, format, NULL);
    } else {
        smm_vasprintf_char(&s, format, args);
    }
    va_end(args);

    t_string_object *str_obj = (t_string_object *)object_alloc_instance(Object_String, 2, strlen(s), s);
    dll_append(dll_args, str_obj);
    object_inc_ref((t_object *)str_obj);

    module_io_method_print(&io_struct, dll_args);

    object_release((t_object *)str_obj);
    smm_free(s);

    return &io_struct;
}
