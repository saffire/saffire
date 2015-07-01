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


void register_external_module(char *path, t_vm_stackframe *frame) {
    void *handle = dlopen(path, RTLD_LAZY);
    if (! handle) return;

    dlerror();
    t_module *module_info = dlsym(handle, "_saffire_module");

    if (! module_info) {
        dlclose(handle);
        return;
    }

    // Register module
    register_module(module_info, path);

    // Register objects inside new frame
    int idx = 0;
    t_object *obj = (t_object *)module_info->objects[idx];
    while (obj != NULL) {
        char *key = vm_context_create_fqcn_from_name("", obj->name);
        DEBUG_PRINT_CHAR("   Registering object: %s\n", key);

        ht_add_str(frame->local_identifiers->data.ht, key, obj);
        object_inc_ref(obj);

        smm_free(key);

        idx++;
        obj = (t_object *)module_info->objects[idx];
    }
}


/**
 * Register an module
 */
int register_module(t_module *mod, const char *path) {
#ifdef __DEBUG
    DEBUG_PRINT_CHAR("   Registering module: %s\n", mod->name);
#endif

    // Add to registered modules list
    t_module_info *module_info = smm_malloc(sizeof(t_module_info));
    module_info->mod = mod;
    module_info->path = string_strdup0(path);
    dll_append(registered_modules, module_info);

    // Initialize module
    mod->init();

    int idx = 0;
    t_object *obj = (t_object *)mod->objects[idx];
    while (obj != NULL) {
        char *key = vm_context_create_fqcn_from_name(mod->name, obj->name);
#ifdef __DEBUG
        DEBUG_PRINT_CHAR("   Registering object: %s\n", key);
#endif
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

static char *core_path = "<core>";
/**
 *
 */
void module_init(void) {
    registered_modules = dll_init();

    // Register core modules
    register_module(&module_sapi_fastcgi, core_path);
    register_module(&module_saffire, core_path);
    register_module(&module_io, core_path);
    register_module(&module_math, core_path);
    register_module(&module_file, core_path);
    register_module(&module_os, core_path);
}

/**
 *
 */
void module_fini(void) {
    // Unregister in the reversed order
    t_dll_element *e = DLL_TAIL(registered_modules);
    while (e) {
        t_module_info *module_info = (t_module_info *)e->data.p;
        unregister_module(module_info->mod);
        smm_free(module_info->path);
        smm_free(module_info);
        e = DLL_PREV(e);
    }
}
