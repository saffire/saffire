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
#include <saffire/vm/codeblock.h>
#include <saffire/vm/context.h>
#include <saffire/general/smm.h>
#include <saffire/debug.h>

/**
 *
 */
static t_vm_codeblock *_vm_codeblock_new(t_bytecode *bytecode, t_vm_context *context) {
    DEBUG_PRINT_CHAR("Adding codeblock with bytecode %08X to context %s (%s)\n", (intptr_t)bytecode, context->module.full, context->file.full);

    t_vm_codeblock *codeblock = smm_malloc(sizeof(t_vm_codeblock));

    // Add context and bytecode to codeblock
    codeblock->bytecode = bytecode;

    // Set context
    codeblock->context = context;

    // Create constants that are located in the bytecode and store inside the codeblock
    codeblock->constants_objects_len = bytecode->constants_len;
    codeblock->constants_objects = smm_malloc(bytecode->constants_len * sizeof(t_object *));
    for (int i=0; i!=codeblock->constants_objects_len; i++) {
        t_object *obj = NULL;
        t_bytecode_constant *c = bytecode->constants[i];
        switch (c->type) {
            case BYTECODE_CONST_CODE :
            {
                // We duplicate the context, so we can free them separately. @TODO: All should point to the same context!
                t_vm_codeblock *child_codeblock = _vm_codeblock_new(bytecode->constants[i]->data.code, vm_context_duplicate(codeblock->context));
                obj = object_alloc(Object_Callable, 3, CALLABLE_CODE_EXTERNAL, child_codeblock, /* arguments */ NULL);
                break;
            }
            case BYTECODE_CONST_STRING :
                obj = object_alloc(Object_String, 2, bytecode->constants[i]->len, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_REGEX :
                obj = object_alloc(Object_Regex, 2, bytecode->constants[i]->len, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                obj = object_alloc(Object_Numerical, 1, bytecode->constants[i]->data.l);
                break;
            default :
                fatal_error(1, "Cannot convert constant type into object!");        /* LCOV_EXCL_LINE */
        }
        codeblock->constants_objects[i] = obj;
        object_inc_ref(obj);
    }

    return codeblock;
}


/**
 * Add a new codeblock
 */
t_vm_codeblock *vm_codeblock_new(t_bytecode *bytecode, t_vm_context *context) {
    t_vm_codeblock *codeblock = _vm_codeblock_new(bytecode, context);

    DEBUG_PRINT_CHAR("NEW CODEBLOCK: %08X (%s)\n", (intptr_t)codeblock, context->file.full);

    return codeblock;
}


/**
 * Destroys a codeblock object. Note that this also cleans up constants that are imported from the codeblock, but does
 * not free the codeblock itself. This is because the codeblock might be a child of another codeblock (like a method
 * inside a regular file codeblock), so it will try and double-free the block. We could add reference counts to it, but
 * for now, we fix this by doing the codeblock release manually.
 */
void vm_codeblock_destroy(t_vm_codeblock *codeblock) {
    if (! codeblock) return;

    // Free constants objects created from the given bytecode. This must be in the reversed order.
    for (int i=codeblock->constants_objects_len-1; i>=0; i--) {
#if __DEBUG_FREE_OBJECT
    DEBUG_PRINT_STRING_ARGS("Freeing constants object: [%08X] %s\n", (intptr_t)codeblock->constants_objects[i], object_debug((t_object *)codeblock->constants_objects[i]));
#endif
        object_release((t_object *)codeblock->constants_objects[i]);
    }
    smm_free(codeblock->constants_objects);

    // Release context
    vm_context_free_context(codeblock->context);

    // Release codeblock itself
    smm_free(codeblock);
}
