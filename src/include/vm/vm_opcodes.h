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
#ifndef __VM_OPCODES_H__
#define __VM_OPCODES_H__

    // No operand opcodes
    #define VM_STOP                 0x00

    #define VM_POP_TOP              0x01
    #define VM_ROT_TWO              0x02
    #define VM_ROT_THREE            0x03
    #define VM_DUP_TOP              0x04
    #define VM_ROT_FOUR             0x05

    #define VM_NOP                  0x09

    #define VM_BINARY_ADD           0x17
    #define VM_BINARY_SUB           0x18
    #define VM_BINARY_MUL           0x19
    #define VM_BINARY_DIV           0x1A
    #define VM_BINARY_SHL           0x1B
    #define VM_BINARY_SHR           0x1C
    #define VM_BINARY_AND           0x1D
    #define VM_BINARY_OR            0x1E
    #define VM_BINARY_XOR           0x1F

    #define VM_INPLACE_ADD           0x20
    #define VM_INPLACE_SUB           0x21
    #define VM_INPLACE_MUL           0x22
    #define VM_INPLACE_DIV           0x23
    #define VM_INPLACE_SHL           0x24
    #define VM_INPLACE_SHR           0x25
    #define VM_INPLACE_AND           0x26
    #define VM_INPLACE_OR            0x27
    #define VM_INPLACE_XOR           0x28

    #define VM_BUILD_CLASS          0x70
    #define VM_MAKE_METHOD          0x71

    #define VM_POP_BLOCK            0x72
    #define VM_RETURN_VALUE         0x73

    #define VM_USE                  0x7E
    #define VM_IMPORT               0x7F

    // One operand opcodes
    #define VM_STORE_ID             0x80
    #define VM_LOAD_CONST           0x81
    #define VM_LOAD_ID              0x82

    #define VM_JUMP_FORWARD         0x83
    #define VM_JUMP_IF_TRUE         0x84
    #define VM_JUMP_IF_FALSE        0x85
    #define VM_JUMP_ABSOLUTE        0x86

    #define VM_DUP_TOPX             0x87

    #define VM_LOAD_GLOBAL          0x88
    #define VM_STORE_GLOBAL         0x89
    #define VM_DELETE_GLOBAL        0x8A

    #define VM_SETUP_LOOP           0x90

    #define VM_CONTINUE_LOOP        0x92
    #define VM_BREAK_LOOP           0x93
    #define VM_BREAKELSE_LOOP       0x94
    #define VM_COMPARE_OP           0x95
    #define VM_SETUP_FINALLY        0x96
    #define VM_SETUP_EXCEPT         0x97
    #define VM_END_FINALLY          0x98



    // Two operand opcodes
    #define VM_CALL_METHOD          0xC0

    // Two operand opcodes
    // Three operand opcodes
    // Four operand opcodes

    // Reserved opcodes
    #define VM_RESERVED             0xFF

#endif