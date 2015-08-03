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
#include <saffire/general/output.h>
#include <saffire/modules/module_api.h>
#include <saffire/general/parse_options.h>
#include <saffire/general/string.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/config.h>
#include <saffire/general/dll.h>
#include <saffire/version.h>
#include <saffire/vm/vm.h>
#include <saffire/vm/thread.h>
#include <saffire/general/smm.h>
#include <string.h>

SAFFIRE_MODULE_METHOD(saffire, get_locale) {
    t_thread *thread = thread_get_current();
    RETURN_STRING_FROM_CHAR(thread->locale);

}

SAFFIRE_MODULE_METHOD(saffire, set_locale) {
    t_string_object *locale_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &locale_obj)) {
        return NULL;
    }

    // Set locale
    t_thread *thread = thread_get_current();
    thread->locale = string_strdup0(STROBJ2CHAR0(locale_obj));

    RETURN_SELF;
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(saffire, version) {
    RETURN_STRING_FROM_CHAR(saffire_version);
}

SAFFIRE_MODULE_METHOD(saffire, gitrev) {
    RETURN_STRING_FROM_CHAR(__GIT_REVISION__);
}

SAFFIRE_MODULE_METHOD(saffire, sapi) {
    if ((vm_runmode & VM_RUNMODE_FASTCGI) == VM_RUNMODE_FASTCGI) {
        RETURN_STRING_FROM_CHAR("fastcgi");
    }
    if ((vm_runmode & VM_RUNMODE_CLI) == VM_RUNMODE_CLI) {
        RETURN_STRING_FROM_CHAR("cli");
    }
    if ((vm_runmode & VM_RUNMODE_REPL) == VM_RUNMODE_REPL) {
        RETURN_STRING_FROM_CHAR("repl");
    }

    RETURN_STRING_FROM_CHAR("unknown");
}

SAFFIRE_MODULE_METHOD(saffire, debug) {
    if ((vm_runmode & VM_RUNMODE_DEBUG) == VM_RUNMODE_DEBUG) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

SAFFIRE_MODULE_METHOD(saffire, args) {
    t_numerical_object *num_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "|n", &num_obj)) {
        RETURN_SELF;
    }

    if (num_obj == NULL) {
        t_hash_table *ht = ht_create();

        for (int idx=0; idx!=saffire_getopt_count(); idx++) {
            ht_add_num(ht, ht->element_count, STR02OBJ(saffire_getopt_string(idx)));
        }
        RETURN_LIST(ht);
    }

    int idx = OBJ2NUM(num_obj);
    if (idx >= saffire_getopt_count() || idx < 0) {
        RETURN_NULL;
    }
    RETURN_STRING_FROM_CHAR(saffire_getopt_string(idx));
}


SAFFIRE_MODULE_METHOD(saffire, exception_handler) {
    t_exception_object *exception_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "o", &exception_obj)) {
        RETURN_SELF;
    }

    if (! OBJECT_IS_EXCEPTION(exception_obj)) {
        RETURN_SELF;
    }

    module_io_print("%s",
                   "                          _   _\n" \
                   "                         | | (_)\n" \
                   "   _____  _____ ___ _ __ | |_ _  ___  _ __\n" \
                   "  / _ \\ \\/ / __/ _ \\ '_ \\| __| |/ _ \\| '_ \\\n" \
                   " |  __/>  < (_|  __/ |_) | |_| | (_) | | | |\n" \
                   "  \\___/_/\\_\\___\\___| .__/ \\__|_|\\___/|_| |_|\n" \
                   "                   | |\n" \
                   "                   |_|\n" \
                   "\n");

    module_io_print("%s", "-------------------------------------------\n");

    module_io_print("  Code: %d\n", exception_obj->data.code);
    module_io_print("  Mesg: %s\n", exception_obj->data.message->val);
    module_io_print("%s", "-------------------------------------------\n");

    t_hash_iter iter;
    ht_iter_init(&iter, exception_obj->data.stacktrace);
    while (ht_iter_valid(&iter)) {
        t_string_object *v = (t_string_object *)(ht_iter_value(&iter));
        module_io_print("%s\n", OBJ2STR0(v));

        ht_iter_next(&iter);
    }

    module_io_print("%s", "-------------------------------------------\n");

    RETURN_SELF;
}


SAFFIRE_MODULE_METHOD(saffire, modules) {
    t_hash_table *modules_ht = ht_create();

    t_dll_element *e = DLL_HEAD(registered_modules);
    while (e) {
        t_module_info *module_info = (t_module_info *)DLL_DATA_PTR(e);

        t_hash_table *module_ht = ht_create();

        // Add all objects
        t_hash_table *objects_ht = ht_create();
        int idx = 0;
        t_object *obj = (t_object *)module_info->mod->objects[idx];
        while (obj != NULL) {
            ht_append_num(objects_ht, STR02OBJ(obj->name));

            idx++;
            obj = (t_object *)module_info->mod->objects[idx];
        }
        ht_add_obj(module_ht, STR02OBJ("objects"), LIST2OBJ(objects_ht));

        // Add module path and description
        ht_add_obj(module_ht, STR02OBJ("path"), STR02OBJ(module_info->path));
        ht_add_obj(module_ht, STR02OBJ("description"), STR02OBJ(module_info->mod->description));

        // Add element to modules hash
        ht_add_obj(modules_ht, STR02OBJ(module_info->mod->name), HASH2OBJ(module_ht));

        e = DLL_NEXT(e);
    }

    RETURN_HASH(modules_ht);
}

t_object saffire_struct = { OBJECT_HEAD_INIT("saffire", objectTypeBase, OBJECT_TYPE_CLASS, NULL, 0), OBJECT_FOOTER };

static void _init(void) {
    saffire_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&saffire_struct, "version",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_version);
    object_add_internal_method((t_object *)&saffire_struct, "git_revision", ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_gitrev);
    object_add_internal_method((t_object *)&saffire_struct, "sapi",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_sapi);
    object_add_internal_method((t_object *)&saffire_struct, "debug",        ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_debug);
    object_add_internal_method((t_object *)&saffire_struct, "set_locale",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_set_locale);
    object_add_internal_method((t_object *)&saffire_struct, "get_locale",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_get_locale);
    object_add_internal_method((t_object *)&saffire_struct, "uncaughtExceptionHandler",   ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_exception_handler);

    object_add_internal_method((t_object *)&saffire_struct, "args",         ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_args);

    object_add_internal_method((t_object *)&saffire_struct, "modules",      ATTRIB_METHOD_STATIC, ATTRIB_VISIBILITY_PUBLIC, module_saffire_method_modules);

    object_add_property((t_object *)&saffire_struct, "fastcgi",    ATTRIB_VISIBILITY_PUBLIC, Object_Null);
    object_add_property((t_object *)&saffire_struct, "cli",        ATTRIB_VISIBILITY_PUBLIC, Object_Null);
    object_add_property((t_object *)&saffire_struct, "repl",       ATTRIB_VISIBILITY_PUBLIC, Object_Null);
}

static void _fini(void) {
    // Destroy methods and properties
    object_free_internal_object(&saffire_struct);
}

static t_object *_objects[] = {
    &saffire_struct,
    NULL
};

t_module module_saffire = {
    "\\saffire",
    "Saffire configuration module",
    _objects,
    _init,
    _fini
};
