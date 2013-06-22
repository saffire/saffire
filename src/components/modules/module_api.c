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
#include <string.h>
#include "general/output.h"
#include "modules/module_api.h"
#include "general/dll.h"
#include "modules/io.h"
#include "modules/saffire.h"
#include "debug.h"
#include "general/hashtable.h"
#include "vm/vm.h"

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))


/**
 * Register an module
 */
int register_module(t_module *mod) {
    DEBUG_PRINT("   Registering module: %s\n", mod->name);

//    t_dll_element *e = DLL_HEAD(modules);
//    while (e) {
//        t_module *src_mod = (t_module *)e->data;
//        if (strcmp(src_mod->name, mod->name) == 0) {
//            // Already registered
//            return 0;
//        }
//        e = DLL_NEXT(e);
//    }

    // Initialize module
    mod->init();

//    t_ns_context *ctx = si_create_context(mod->name);
//    dll_append(modules, mod);

    int idx = 0;
    t_object *obj = (t_object *)mod->objects[idx];
    while (obj != NULL) {
        char key[100]; // @TODO: fixme
        sprintf(key, "%s::%s", mod->name, obj->name);

        vm_populate_builtins(key, obj);

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

    // @TODO: Remove from builtins or so

    // Nothing found
    return 0;
}

/**
 *
 */
void module_init(void) {
    modules = dll_init();
    register_module(&module_saffire);
    register_module(&module_io);
}

/**
 *
 */
void module_fini(void) {
    unregister_module(&module_saffire);
    unregister_module(&module_io);
    dll_free(modules);
}
