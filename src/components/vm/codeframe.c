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
#include "vm/codeframe.h"
#include "vm/context.h"
#include "general/smm.h"
#include "debug.h"

t_hash_table *codeframes;    // Hash table with all code frames


/**
 *
 */
static t_vm_codeframe *_vm_codeframe_new(t_bytecode *bytecode, t_vm_context *context) {
    t_vm_codeframe *codeframe = smm_malloc(sizeof(t_vm_codeframe));

    // Add context and bytecode to codeframe
    codeframe->bytecode = bytecode;
    codeframe->context = context;

    // Create constants that are located in the bytecode and store inside the codeframe
    codeframe->constants_objects = smm_malloc(bytecode->constants_len * sizeof(t_object *));
    for (int i=0; i!=bytecode->constants_len; i++) {
        t_object *obj = NULL;
        t_bytecode_constant *c = bytecode->constants[i];
        switch (c->type) {
            case BYTECODE_CONST_CODE :
            {
                // We create a reference to the source filename of the original bytecode name.
                //bytecode->constants[i]->data.code->source_filename = string_strdup0(bytecode->source_filename);
                t_vm_codeframe *child_codeframe = vm_codeframe_addchild(codeframe, bytecode->constants[i]->data.code);
                obj = object_alloc(Object_Callable, 3, CALLABLE_CODE_EXTERNAL, child_codeframe, /* arguments */ NULL);
                break;
            }
            case BYTECODE_CONST_STRING :
                obj = object_alloc(Object_String, 2, strlen(bytecode->constants[i]->data.s), bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_REGEX :
                obj = object_alloc(Object_Regex, 2, strlen(bytecode->constants[i]->data.r), bytecode->constants[i]->data.r);
                break;
            case BYTECODE_CONST_NUMERICAL :
                obj = object_alloc(Object_Numerical, 1, bytecode->constants[i]->data.l);
                break;
            default :
                fatal_error(1, "Cannot convert constant type into object!");        /* LCOV_EXCL_LINE */
        }
        codeframe->constants_objects[i] = obj;
    }

    return codeframe;
}


/**
 * Add a codeframe that is actually a child of another codeframe
 */
t_vm_codeframe *vm_codeframe_addchild(t_vm_codeframe *parent_codeframe, t_bytecode *bytecode) {
    t_vm_codeframe *codeframe = _vm_codeframe_new(bytecode, parent_codeframe->context);

    return codeframe;
}

/**
 * Add a new codeframe
 */
t_vm_codeframe *vm_codeframe_new(t_bytecode *bytecode, t_vm_context *context) {
    t_vm_codeframe *codeframe = _vm_codeframe_new(bytecode, context);

    ht_add_str(codeframes, context->class.full, codeframe);

    return codeframe;
}



/**
 *
 */
t_vm_codeframe *vm_codeframe_find(char *class_path) {
    t_vm_codeframe *codeframe = ht_find_str(codeframes, class_path);
    DEBUG_PRINT_CHAR(" * *** Looking for a frame in cache with key '%s': %s\n", class_path, codeframe ? "Found" : "Nothing found");

    return codeframe;
}


/**
 *
 */
void vm_codeframe_destroy(t_vm_codeframe *codeframe) {
    if (! codeframe) return;

    if (codeframe->bytecode) {
        // Free constants objects
        for (int i=0; i!=codeframe->bytecode->constants_len; i++) {
            DEBUG_PRINT_STRING(char0_to_string("Freeing: %s\n"), object_debug((t_object *)codeframe->constants_objects[i]));
            object_release((t_object *)codeframe->constants_objects[i]);
        }
        smm_free(codeframe->constants_objects);

        // Release bytecode
        bytecode_free(codeframe->bytecode);
    }

    // Release context
    vm_context_free_context(codeframe);

    // Release codeframe itself
    smm_free(codeframe);
}

/**
 *
 */
void vm_codeframe_init(void) {
    codeframes = ht_create();
}

/**
 *
 */
void vm_codeframe_fini(void) {
    // Release all code frames
    t_hash_iter iter;
    ht_iter_init(&iter, codeframes);
    while (ht_iter_valid(&iter)) {
        t_vm_codeframe *codeframe = ht_iter_value(&iter);

        DEBUG_PRINT_CHAR("DESTROYING CODEFRAME: %08lX (%s)\n", (unsigned long)codeframe, codeframe->context->file.path);

        vm_codeframe_destroy(codeframe);

        ht_iter_next(&iter);
    }

    ht_destroy(codeframes);
}
