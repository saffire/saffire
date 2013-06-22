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
#ifndef __VM_BLOCK_H__
#define __VM_BLOCK_H__

    #define BLOCK_MAX_DEPTH             20          // This is the same depth as defined in python

    #define BLOCK_TYPE_LOOP             1
    #define BLOCK_TYPE_EXCEPTION        2

    typedef struct _vm_frameblock {
        int type;       // Type (any of the BLOCK_TYPE_*)
        union {
            struct {
                int ip;         // Saved instruction pointer
                int ip_else;    // Saved instruction pointer to ELSE part
            } loop;
            struct {
                int ip_catch;           // Saved instruction pointer to CATCH blocks
                int ip_finally;         // Saved instruction pointer to FINALLY block
                int ip_end_finally;     // Saved instruction pointer to the end of FINALLY block

                int in_finally;         // 1: We are currently handling the finally block
            } exception;
        } handlers;
        int sp;         // Saved stack pointer
        int visited;    // When !=0, this frame is already visited by a JUMP_IF_*_AND_FIRST
    } t_vm_frameblock;

    struct _vm_frame;

    void vm_push_block_loop(struct _vm_frame *frame, int type, int sp, int ip, int ip_else);
    void vm_push_block_exception(struct _vm_frame *frame, int type, int sp, int ip_catch, int ip_finally, int ip_end_finally);
    t_vm_frameblock *vm_pop_block(struct _vm_frame *frame);
    t_vm_frameblock *vm_fetch_block(struct _vm_frame *frame);

#endif


