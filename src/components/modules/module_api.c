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
#include <string.h>
#include <dlfcn.h>
#include <saffire/general/output.h>
#include <saffire/modules/module_api.h>
#include <saffire/general/dll.h>
#include <saffire/debug.h>
#include <saffire/general/hashtable.h>
#include <saffire/vm/vm.h>
#include <saffire/general/smm.h>
#include <saffire/modules/modules.h>

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))


t_dll *registered_modules;

void register_external_module(char *path) {
    void *handle = dlopen(path, RTLD_LAZY);
    if (! handle) return;

    dlerror();
    t_module *module_info = dlsym(handle, "_saffire_module");

    if (! module_info) {
        dlclose(handle);
        return;
    }

    register_module(module_info);
}

/**
 * Register an module
 */
int register_module(t_module *mod) {
    DEBUG_PRINT_CHAR("   Registering module: %s\n", mod->name);

    // Add to registered modules list
    dll_append(registered_modules, mod);

    // Initialize module
    mod->init();

    int idx = 0;
    t_object *obj = (t_object *)mod->objects[idx];
    while (obj != NULL) {
        char *key;
        vm_context_create_fqcn(mod->name, obj->name, &key);
        DEBUG_PRINT_CHAR("   Registering object: %s\n", key);
        vm_populate_builtins(key, obj);
        smm_free(key);

        object_inc_ref(obj);

        idx++;
        obj = (t_object *)mod->objects[idx];
    }
    return 1;
}

/**
 * Unregister
 */
int unregister_module(t_module *mod) {
    // Fini module
    mod->fini();

    int idx = 0;
    t_object *obj = (t_object *)mod->objects[idx];
    while (obj != NULL) {
        object_release(obj);
        idx++;
        obj = (t_object *)mod->objects[idx];
    }

    // Nothing found
    return 0;
}

/**
 *
 */
void module_init(void) {
    registered_modules = dll_init();

    register_module(&module_sapi_fastcgi);
    register_module(&module_saffire);
    register_module(&module_io);
    register_module(&module_math);
    register_module(&module_file);

    register_external_module("./modules/exif/exif.so");
}

/**
 *
 */
void module_fini(void) {
    // Unregister in the reversed order
    t_dll_element *e = DLL_TAIL(registered_modules);
    while (e) {
        t_module *mod = (t_module *)e->data.p;
        unregister_module(mod);
        e = DLL_PREV(e);
    }
}
