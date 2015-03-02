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

#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <general/output.h>
#include <compiler/bytecode.h>
#include <general/smm.h>
#include <general/config.h>
#include <general/hashtable.h>
#include <compiler/output/asm.h>
#include <compiler/ast_to_asm.h>




static void _free_constant(t_bytecode_constant *c) {
     switch (c->type) {
         case BYTECODE_CONST_STRING :
         case BYTECODE_CONST_REGEX :
            smm_free(c->data.s);
            break;
         case BYTECODE_CONST_NUMERICAL :
             break;
         case BYTECODE_CONST_CODE :
             bytecode_free(c->data.code);
             break;
         default :
             fatal_error(1, "Unknown constant type %d\n", c->type); /* LCOV_EXCL_LINE */
     }
}


/**
 * Add constant to a bytecode structure
 */
static void _add_constant(t_bytecode *bc, t_bytecode_constant *c) {
    bc->constants = smm_realloc(bc->constants, sizeof(t_bytecode_constant *) * (bc->constants_len + 1));
    bc->constants[bc->constants_len] = c;
    bc->constants_len++;
}



/**
 * Add a new string constant to the bytecode structure
 */
static void _new_constant_string(t_bytecode *bc, char *s, int len) {
    // Setup constant
    t_bytecode_constant *c = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
    c->type = BYTECODE_CONST_STRING;
    c->len = len;
    c->data.s = smm_malloc(len);
    memcpy(c->data.s, s, len);

    _add_constant(bc, c);
}

/**
 * Add a new regex constant to the bytecode structure
 */
static void _new_constant_regex(t_bytecode *bc, char *s, int len) {
    // Setup constant
    t_bytecode_constant *c = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
    c->type = BYTECODE_CONST_REGEX;
    c->len = len;
    c->data.s = smm_malloc(len);
    memcpy(c->data.s, s, len);

    _add_constant(bc, c);
}


/**
 * Add a new string constant to the bytecode structure
 */
static void _new_constant_code(t_bytecode *bc, t_bytecode *child_bc) {
    // Setup constant
    t_bytecode_constant *c = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
    c->type = BYTECODE_CONST_CODE;
    c->len = 0;
    c->data.code = child_bc;

    _add_constant(bc, c);
}


/**
 * Add a new constant to the bytecode structure
 */
static void _new_constant_long(t_bytecode *bc, long l) {
    // Setup constant
    t_bytecode_constant *c = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
    c->type = BYTECODE_CONST_NUMERICAL ;
    c->len = sizeof(long);
    c->data.l = l;

    _add_constant(bc, c);
}


/**
 * Add a new identifier to the bytecode structure
 */
static void _new_name(t_bytecode *bc, char *var) {
    // Setup identifier
    t_bytecode_identifier *c = smm_malloc(sizeof(t_bytecode_identifier));
    c->len = strlen(var);
    c->s = string_strdup0(var);

    // Add identifier
    bc->identifiers = smm_realloc(bc->identifiers, sizeof(t_bytecode_identifier *) * (bc->identifiers_len + 1));
    bc->identifiers[bc->identifiers_len] = c;
    bc->identifiers_len++;
}



/**
 * Read from "buffer" on offset "*bufptr". Read "size" bytes and store inside "data".
 * Bufptr will be automatically increased to the next offset.
 */
static void _read_buffer(char *buf, int *bufptr, int size, void *data) {
    memcpy(data, buf+*bufptr, size);
    *bufptr += size;
}


/**
 * Write "size" bytes from "data" into "buffer" on offset "*bufptr". Note that "buffer" can point to a NULL value, in
 * which case a new buffer will be allocated. This method takes care of enough space in the buffer through reallocs
 * Bufptr will be automatically increased to the next offset.
 */
static void _write_buffer(char **buf, int *bufptr, int size, void *data) {
    *buf = smm_realloc(*buf, *bufptr + size);
    memcpy(*buf + *bufptr, data, size);
    *bufptr += size;
}


/**
 * Converts a binary stream to a bytecode structure
 */
t_bytecode *bytecode_unmarshal(char *bincode) {
    int pos = 0;
    char *s; long l; int j;
    int clen, vlen;
    t_bytecode *child_bytecode;

    // Initialize new bytecode structure
    t_bytecode *bytecode = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
    bzero(bytecode, sizeof(t_bytecode));
    bytecode->identifiers = NULL;

    // Read headers
    _read_buffer(bincode, &pos, sizeof(uint32_t), &bytecode->stack_size);
    _read_buffer(bincode, &pos, sizeof(uint32_t), &bytecode->code_len);

    // Allocate memory for code and store
    bytecode->code = smm_malloc(bytecode->code_len);
    _read_buffer(bincode, &pos, bytecode->code_len, bytecode->code);

    // Read all identifiers
    _read_buffer(bincode, &pos, sizeof(int), &vlen);
    for (int i=0; i!=vlen; i++) {
        _read_buffer(bincode, &pos, sizeof(int), &j);

        // identifier strings do not have a trailing \0 on disk.
        s = smm_malloc(j+1);
        _read_buffer(bincode, &pos, j, s);
        s[j] = '\0';
        _new_name(bytecode, s);
        smm_free(s);
    }

    // Read constants
    _read_buffer(bincode, &pos, sizeof(uint32_t), &clen);
    for (int i=0; i!=clen; i++) {
        char type; int len;
        _read_buffer(bincode, &pos, sizeof(char), &type);
        _read_buffer(bincode, &pos, sizeof(int), &len);

        switch (type) {
            case BYTECODE_CONST_STRING :
                // Constant strings do not have a trailing \0 on disk.
                s = smm_malloc(len+1);
                _read_buffer(bincode, &pos, len, s);
                s[len] = '\0';
                _new_constant_string(bytecode, s, len);
                smm_free(s);
                break;
            case BYTECODE_CONST_REGEX :
                // Constant regex do not have a trailing \0 on disk.
                s = smm_malloc(len+1);
                _read_buffer(bincode, &pos, len, s);
                s[len] = '\0';
                _new_constant_regex(bytecode, s, len);
                smm_free(s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                _read_buffer(bincode, &pos, len, &l);
                _new_constant_long(bytecode, l);
                break;
            case BYTECODE_CONST_CODE :
                // Read binary buffer to new bytecode
                child_bytecode = bytecode_unmarshal(bincode+pos);
                pos += len; // Skip the just read binary bytecode
                _new_constant_code(bytecode, child_bytecode);
                break;
            default :
                fatal_error(1, "Unknown constant type %d\n", type); /* LCOV_EXCL_LINE */
        }
    }

    // Read lineno's
    _read_buffer(bincode, &pos, sizeof(int), &bytecode->lino_offset);
    _read_buffer(bincode, &pos, sizeof(int), &bytecode->lino_length);
    bytecode->lino = NULL;
    if (bytecode->lino_length > 0) {
        bytecode->lino = smm_malloc(bytecode->lino_length);
        _read_buffer(bincode, &pos, bytecode->lino_length, bytecode->lino);
    }

    return bytecode;
}


/**
 * Convert bytecode structure into a binary stream (NOTE: bincode is an unallocated pointer!)
 */
int bytecode_marshal(t_bytecode *bytecode, int *bincode_off, char **bincode) {
    char *child_bincode = NULL;
    int child_bincode_len = 0;

    // Write headers and codeblock
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->stack_size);
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->code_len);
    _write_buffer(bincode, bincode_off, bytecode->code_len, bytecode->code);

    // Write identifiers
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->identifiers_len);
    for (int i=0; i!=bytecode->identifiers_len; i++) {
        _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->identifiers[i]->len);
        _write_buffer(bincode, bincode_off, bytecode->identifiers[i]->len, bytecode->identifiers[i]->s);
    }

    // Write constants
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants_len);
    for (int i=0; i!=bytecode->constants_len; i++) {
        _write_buffer(bincode, bincode_off, sizeof(char), &bytecode->constants[i]->type);
        switch (bytecode->constants[i]->type) {
            case BYTECODE_CONST_CODE :
                child_bincode_len = 0;
                bytecode_marshal(bytecode->constants[i]->data.code, &child_bincode_len, &child_bincode);
                _write_buffer(bincode, bincode_off, sizeof(int), &child_bincode_len);
                _write_buffer(bincode, bincode_off, child_bincode_len, child_bincode);
                break;
            case BYTECODE_CONST_STRING :
            case BYTECODE_CONST_REGEX :
                _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants[i]->len);
                _write_buffer(bincode, bincode_off, bytecode->constants[i]->len, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants[i]->len);
                _write_buffer(bincode, bincode_off, bytecode->constants[i]->len, &bytecode->constants[i]->data.l);
                break;
            default :
                fatal_error(1, "Unknown constant type %d\n", bytecode->constants[i]->type); /* LCOV_EXCL_LINE */
        }
    }

    // Write linenumber offsets
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->lino_offset);
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->lino_length);
    _write_buffer(bincode, bincode_off, bytecode->lino_length, bytecode->lino);

    return 1;
}



/**
 * Free allocated bytecode structure
 */
void bytecode_free(t_bytecode *bc) {
    smm_free(bc->code);

    for (int i=0; i!=bc->constants_len; i++) {
        _free_constant(bc->constants[i]);
        smm_free(bc->constants[i]);
    }
    smm_free(bc->constants);

    for (int i=0; i!=bc->identifiers_len; i++) {
        t_bytecode_identifier *id = bc->identifiers[i];
        smm_free(id->s);
        smm_free(bc->identifiers[i]);
    }
    smm_free(bc->identifiers);

    smm_free(bc->lino);

    smm_free(bc);
}



/**
 *
 */
t_bytecode *convert_frames_to_bytecode(t_hash_table *frames, char *name, int startline) {
    t_dll_element *e;

    // Seek frame
    t_asm_frame *frame = ht_find_str(frames, name);
    if (! frame) return NULL;

    // Create bytecode structure
    t_bytecode *bc = smm_malloc(sizeof(t_bytecode));
    bzero(bc, sizeof(t_bytecode));

    // Fill initial values
    bc->stack_size = frame->stack_size;
    bc->code_len = frame->code_len;
    bc->code = smm_malloc(bc->code_len);
    memcpy(bc->code, frame->code, bc->code_len);

    bc->lino_offset = startline;
    bc->lino_length = frame->lino_len;
    bc->lino = smm_malloc(bc->lino_length);
    memcpy(bc->lino, frame->lino, bc->lino_length);

    // Add constants (order matter!)
    e = DLL_HEAD(frame->constants);
    while (e) {
        t_asm_constant *c = (t_asm_constant *)e->data;
        switch (c->type) {
            case const_code :
                _new_constant_code(bc, convert_frames_to_bytecode(frames, c->data.s, 1));
                break;
            case const_string :
                _new_constant_string(bc, c->data.s, strlen(c->data.s));
                break;
            case const_regex :
                _new_constant_regex(bc, c->data.s, strlen(c->data.s));
                break;
            case const_long :
                _new_constant_long(bc, c->data.l);
                break;
        }

        e = DLL_NEXT(e);
    }

    // Add identifiers (order matter!)
    e = DLL_HEAD(frame->identifiers);
    while (e) {
        _new_name(bc, (char *)e->data);
        e = DLL_NEXT(e);
    }

    return bc;
}
