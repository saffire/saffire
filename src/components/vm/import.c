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
const char *module_search_paths[] = {
    ".",
    "/usr/share/saffire/modules",
    "/usr/share/saffire/modules/sfl",
    NULL
};


/**
 * Executes a source-file inside a frame, and extracts the needed class from the frame
 *
 * @param frame
 * @param source_file
 * @param class
 * @param module_frame
 * @return
 */
static t_vm_frame *_execute_frame(t_vm_frame *frame, char *source_file) {

    // @TODO: Don't care about cached bytecode for now! Compile source to BC
    t_ast_element *ast = ast_generate_from_file(source_file);
    t_hash_table *asm_code = ast_to_asm(ast, 1);
    t_bytecode *bc = assembler(asm_code, source_file);
    bc->source_filename = smm_strdup(source_file);

    // Create a new frame and run it!
    t_vm_frame *module_frame = vm_frame_new(frame, "{import}", bc);
    vm_execute(module_frame);

    DEBUG_PRINT("\n\n\n\n * End of running module bytecode.\n");

    return module_frame;
}

/**
 * Build actual class-path from a module (Framework::View -> Framework/View)
 *
 * @param path
 * @return
 */
static char *build_class_path(char *module) {
    char *class_path = smm_strdup(module);
    char *c;

    // We find all '::', replace the first character with '/', and move all
    // remaining chars in the string up one position
    while (c = strstr(class_path, "::"), c != NULL) {
        *c = '/';
        memmove(c+1, c+2, strlen(c)+1);
    }

    return class_path;
}

/**
 * Construct a absolute path from a root and sub path.
 *
 * @param root_path
 * @param sub_path
 * @return
 */
static char *construct_import_path(char *root_path, char *sub_path) {
    char *path = realpath(root_path, NULL);
    if (! path) return NULL;

    char *final_path;
    asprintf(&final_path, "%s/%s.sf", root_path, sub_path);
    DEBUG_PRINT(" * *** Constructed path: '%s'\n", final_path);
    return final_path;
}


/**
 * Search all import paths for class_path, and extracts+caches the needed class from that file
 *
 * @param frame
 * @param class_path
 * @param class
 * @param file_found
 * @return
 */
static t_object *search_import_paths(t_vm_frame *frame, char *class_path, char *class, int *file_found) {
    *file_found = 0;

    char **search_root_path = (char **)&module_search_paths;
    while (*search_root_path) {
        char *final_search_path = construct_import_path(*search_root_path, class_path);

        *file_found = is_file(final_search_path) ? 1 : 0;

        if (*file_found) {
            t_vm_frame *import_frame = _execute_frame(frame, final_search_path);
            t_object *obj = vm_frame_find_identifier(import_frame, class);

            // Add frame to cache
            DEBUG_PRINT(" * *** Adding to import cache at key '%s'\n", class_path);
            ht_add(import_cache, class_path, import_frame);

            // free instead of smm_free, since construct_import_path() allocates through asprintf()
            free(final_search_path);

            return obj;
        }

        // free instead of smm_free, since construct_import_path() allocates through asprintf()
        free(final_search_path);

        // Check next root path
        search_root_path++;
    }

    return NULL;
}


/**
 *
 */
t_object *vm_import(t_vm_frame *frame, char *module, char *class) {
    t_object *obj = NULL;
    int file_found = 0;

    DEBUG_PRINT("Importing class '%s' from module '%s'\n", class, module);
    char *fqcn = smm_malloc(strlen(module) + strlen(class) + strlen("::") + 1);
    sprintf(fqcn, "%s::%s", module, class);
    DEBUG_PRINT(" * Looks like we're looking for '%s'\n", fqcn);
    obj = vm_frame_find_identifier(frame, fqcn);
    smm_free(fqcn);
    // Found our object, return
    if (obj != NULL) {
        DEBUG_PRINT(" * *** Found as object inside our frame\n");
        return obj;
    }



    DEBUG_PRINT(" * *** Importing file. Scanning searchpath:\n");
    char *class_path = build_class_path(module);

    // CHeck if we already executed the frame before. If so, it's stored in cache
    t_vm_frame *cached_frame = ht_find(import_cache, class_path);
    DEBUG_PRINT(" * *** Looking for a frame in cache that holds '%s': %s\n", class_path, cached_frame ? "Found" : "Nothing found");
    if (cached_frame) {
        // Fetch the object from the frame
        obj = vm_frame_find_identifier(cached_frame, class);
        file_found = 1;
    } else {
        obj = search_import_paths(frame, class_path, class, &file_found);
    }

    smm_free(class_path);

    // No matching file found
    if (! file_found) {
        object_raise_exception(Object_SystemException, "Cannot find module '%s'", module);
        return NULL;
    }

    // No object found in file
    if (! obj) {
        object_raise_exception(Object_ImportException, "Cannot find class '%s' in module '%s'", class, module);
        return NULL;
    }

    // All ok
    return obj;
}