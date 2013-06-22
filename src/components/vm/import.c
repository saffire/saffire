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
#include <limits.h>
#include "vm/vm.h"
#include "vm/frame.h"
#include "objects/objects.h"
#include "compiler/ast_nodes.h"
#include "compiler/ast_to_asm.h"
#include "compiler/output/asm.h"
#include "general/path_handling.h"
#include "general/output.h"
#include "debug.h"


/*
 * @TODO: Modules are search in this order on these paths. They are currently hardcoded
 */
const char *module_search_path[] = {
    ".",
    "/usr/share/saffire/modules",
    "/usr/share/saffire/modules/sfl",
    NULL
};


static t_object *_import_class_from_file(t_vm_frame *frame, char *source_file, char *class) {
    DEBUG_PRINT(" * _import_class_from_file(%s, %s)\n", source_file, class);

    // @TODO: Don't care about cached bytecode for now! Compile source to BC
    t_ast_element *ast = ast_generate_from_file(source_file);
    t_hash_table *asm_code = ast_to_asm(ast, 1);
    t_bytecode *bc = assembler(asm_code, source_file);
    bc->source_filename = smm_strdup(source_file);

    // Create a new frame and run it!
    t_vm_frame *module_frame = vm_frame_new(frame, "{import}", bc);
    vm_execute(module_frame);

    DEBUG_PRINT("\n\n\n\n * End of running module bytecode.\n");

    t_object *obj = vm_frame_find_identifier(module_frame, class);
    DEBUG_PRINT("FOUND: %08X\n", (unsigned int)obj);
    return obj;
}

/**
 *
 */
t_object *vm_import(t_vm_frame *frame, char *module, char *class) {
    DEBUG_PRINT("\n\n\n");
    DEBUG_PRINT("*** looks like we need to import class '%s' from module '%s'\n", class, module);

    // Allocate room to build our complete namespace string
    char *fqcn = smm_malloc(strlen(module) + strlen(class) + strlen("::") + 1);
    sprintf(fqcn, "%s::%s", module, class);
    DEBUG_PRINT(" * Looks like we're looking for '%s'\n", fqcn);
    t_object *obj = vm_frame_find_identifier(frame, fqcn);
    smm_free(fqcn);

    // Found our object, return
    if (obj != NULL) {
        return obj;
    }

    // Looks like we haven't found it. Let's try and load it from disk.
    DEBUG_PRINT(" * *** Nothing found in current frame. Scanning searchpath:\n");

    // Scan . and /usr/saffire/modules only!
    char **current_search_path = (char **)&module_search_path;
    while (*current_search_path) {
        DEBUG_PRINT("   * Searching on path '%s'\n", *current_search_path);
        char final_path[PATH_MAX];
        char *path = realpath(*current_search_path, NULL);
        if (path) {
            snprintf(final_path, PATH_MAX, "%s/%s.sf", path, class);
            DEBUG_PRINT("   * Looking for module at path '%s'\n", final_path);

            if (is_file(final_path)) {
                DEBUG_PRINT("   * Found a matching file. Whoohoo!\n");
                t_object *obj = _import_class_from_file(frame, final_path, class);
                if (! obj) {
                    object_raise_exception(Object_ImportException, "Cannot find class '%s' in module '%s'", class, module);
                    return NULL;
                }
                return obj;
            } else {
                DEBUG_PRINT("   * Nothing found here.. continuing..\n");
            }
        } else {
            DEBUG_PRINT("   * This path is not real!?");
        }
        current_search_path++;
    }

    object_raise_exception(Object_SystemException, "Cannot find module '%s'", module);
    return NULL;
}
