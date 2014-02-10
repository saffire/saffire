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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "general/config.h"
#include "vm/thread.h"
#include "general/smm.h"


// Current running thread. Don't change directory, but only through thread_switch() methods.
t_thread *current_thread;


t_thread *thread_new(void) {
    t_thread *thread = smm_malloc(sizeof(t_thread));
    bzero(thread, sizeof(t_thread));

    thread->locale = config_get_string("intl.locale", "nl_NL");
    return thread;
}

void thread_free(t_thread *thread) {
    if (thread->locale) {
        smm_free(thread->locale);
    }

    smm_free(thread);
}
/**
 * Returns the running frame inside the current thread
 */
t_vm_stackframe *thread_get_current_frame() {
    return current_thread->frame;
}

t_thread *thread_get_current() {
    return current_thread;
}

/**
 * Sets the frame inside the current thread
 */
void thread_set_current_frame(t_vm_stackframe *frame) {
    current_thread->frame = frame;
}

/**
 * returns 1 when an exception has been thrown in this thread. 0 otherwise
 */
int thread_exception_thrown(void) {
    return (current_thread->exception != NULL);
}

/**
 * Creates a new exception based on the base class, on the code and message given
 */
void thread_create_exception(t_exception_object *exception, int code, const char *message) {
    current_thread->exception = (t_exception_object *)object_alloc((t_object *)exception, 2, code, char0_to_string(message));
}

/**
 * Sets the current exception
 */
void thread_set_exception(t_exception_object *exception) {
    current_thread->exception = exception;
}


/**
 * vararg version of thread_create_exception
 */
void thread_create_exception_printf(t_exception_object *exception, int code, const char *format, ...) {
    va_list args;
    char *buf;

    va_start(args, format);
    smm_vasprintf_char(&buf, format, args);
    va_end(args);

    thread_create_exception(exception, code, buf);
    smm_free(buf);
}


/**
 * Returns exception object for the current thread, or NULL when no exception has been raised
 */
t_exception_object *thread_get_exception(void) {
    return (t_exception_object *)current_thread->exception;
}
