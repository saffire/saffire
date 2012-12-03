/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
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
#include "vm/block.h"
#include "vm/frame.h"
#include "debug.h"

/**
 *
 */
void vm_push_block(t_vm_frame *frame, int type, int ip, int sp, int ip_else) {
    t_vm_frameblock *block;

    DEBUG_PRINT(">>> PUSH BLOCK [%d] (%04X %04X)\n", frame->block_cnt, ip, ip_else);

    if (frame->block_cnt >= BLOCK_MAX_DEPTH) {
        printf("Too many blocks!");
        exit(1);
    }

    block = &frame->blocks[frame->block_cnt];
    frame->block_cnt++;

    block->type = type;
    block->ip = ip;
    block->ip_else = ip_else;       // Else IP for while/else
    block->sp = sp;
    block->visited = 0;
}


/**
 *
 */
t_vm_frameblock *vm_pop_block(t_vm_frame *frame) {
    t_vm_frameblock *block;

    DEBUG_PRINT(">>> POP BLOCK [%d] \n", frame->block_cnt);

    if (frame->block_cnt <= 0) {
        printf("Not enough blocks!");
        exit(1);
    }

    frame->block_cnt--;
    block = &frame->blocks[frame->block_cnt];
    return block;
}

/**
 *
 */
t_vm_frameblock *vm_fetch_block(t_vm_frame *frame) {
    DEBUG_PRINT(">>> FETCH BLOCK\n");
    return &frame->blocks[frame->block_cnt - 1];
}