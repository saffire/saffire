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
#include "general/stack.h"
#include "general/dll.h"
#include "general/smm.h"
#include "general/output.h"

/**
 *
 */
t_stack *stack_init(void) {
    t_stack *stack = (t_stack *)smm_malloc(sizeof(t_stack));

    stack->dll = dll_init();
    return stack;
}


/**
 *
 */
void stack_push(t_stack *stack, void *data) {
    dll_append(stack->dll, data);
}


/**
 *
 */
void *stack_pop(t_stack *stack) {
    if (stack->dll->size <= 0) {
        fatal_error(1, "cannot pop from an empty stack!\n");
    }
    t_dll_element *e = DLL_TAIL(stack->dll);

    dll_remove(stack->dll, e);
    return e->data;
}

/**
 *
 */
void *stack_peek(t_stack *stack) {
    t_dll_element *e = DLL_TAIL(stack->dll);
    if (! e) return NULL;
    return e->data;
}

/**
 *
 */
int stack_size(t_stack *stack) {
    return stack->dll->size;
}


/**
 *
 */
void stack_free(t_stack *stack) {
    dll_free(stack->dll);
    smm_free(stack);
}
