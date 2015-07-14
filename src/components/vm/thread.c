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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <saffire/general/config.h>
#include <saffire/vm/thread.h>
#include <saffire/general/smm.h>
#include <saffire/vm/vm.h>


// Current running thread. Don't change directly, but only through thread_switch() methods.
t_thread *current_thread;


t_thread *thread_new(void) {
    t_thread *thread = smm_malloc(sizeof(t_thread));
    bzero(thread, sizeof(t_thread));

    thread->locale = string_strdup0(config_get_string("intl.locale", "nl_NL"));
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

t_vm_stackframe *thread_get_exception_frame(void) {
    return current_thread->exception_frame;
}

/**
 * returns 1 when an exception has been thrown in this thread. 0 otherwise
 */
int thread_exception_thrown(void) {
    return (current_thread->exception != NULL);
}

int getlineno(t_vm_stackframe *frame);



t_hash_table *thread_create_stacktrace() {
    // Create stack trace array
    t_hash_table *stacktrace = ht_create();

    t_vm_stackframe *frame = thread_get_current_frame();
    int depth = 0;
    while (frame) {
        char *s = NULL;
        t_vm_context *ctx = vm_frame_get_context(frame);
        smm_asprintf_char(&s, "#%d %s:%d %s.%s (<args>)",
            depth,
            ctx->file.full ? ctx->file.full : "<none>",
            getlineno(frame),
            ctx->module.full ? ctx->module.full : "",
            frame->trace_method ? frame->trace_method : ""
        );

        t_string_object *str = (t_string_object *)object_alloc_instance(Object_String, 2, strlen(s), s);
        ht_add_num(stacktrace, stacktrace->element_count, str);

        frame = frame->parent;
        depth++;
    }

    return stacktrace;
}

/**
 * Creates a new exception based on the base class, on the code and message given
 */
void thread_create_exception(t_exception_object *exception, int code, const char *message) {
    t_hash_table *stacktrace = thread_create_stacktrace();

    current_thread->exception = (t_exception_object *)object_alloc_instance((t_object *)exception, 3, code, char0_to_string(message), stacktrace);
    current_thread->exception_frame = thread_get_current_frame();

    object_inc_ref((t_object *)current_thread->exception);
}

void thread_clear_exception() {
    if (current_thread->exception) {
        object_release((t_object *)current_thread->exception);
    }
    current_thread->exception = NULL;
}

/**
 * Sets the current exception
 */
void thread_set_exception(t_exception_object *exception) {
    current_thread->exception = exception;
    object_inc_ref((t_object *)exception);

    // Add stack trace
    t_hash_table *stacktrace = thread_create_stacktrace();
    exception->data.stacktrace = stacktrace;
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

t_exception_object *thread_save_exception(void) {
    t_exception_object *exception_obj = (t_exception_object *)current_thread->exception;

    if (! exception_obj) return exception_obj;

    // save guard exception for releasing
    object_inc_ref((t_object *)exception_obj);

    thread_clear_exception();

    return exception_obj;
}

void thread_restore_exception(t_exception_object *exception) {
    if (! exception) return;

    thread_set_exception(exception);

    // Release the lock from our temporary stored exception
    object_release((t_object *)exception);
}

void thread_dump_exception(t_exception_object *exception) {
    if (! exception) return;

    object_release((t_object *)exception);
}
