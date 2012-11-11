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
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "compiler/bytecode.h"
#include "compiler/ast.h"
#include "general/dll.h"
#include "general/smm.h"
#include "version.h"
#include "vm/vm_opcodes.h"
#include "interpreter/errors.h"


/**
 * Add a new constant to the bytecode structure
 */
static void _new_constant(t_bytecode *bc, int type, int len, void *data) {
    t_bytecode_constant *c = smm_malloc(sizeof(t_bytecode_constant));
    c->type = type;
    c->len = len;
    c->data.ptr = (void *)data;

    bc->constants = smm_realloc(bc->constants, sizeof(t_bytecode_constant *) * (bc->constants_len + 1));
    bc->constants[bc->constants_len] = c;
    bc->constants_len++;
}


/**
 * Add a new variable to the bytecode structure
 */
static void _new_variable(t_bytecode *bc, char *var) {
    t_bytecode_variable *c = smm_malloc(sizeof(t_bytecode_variable));
    c->len = strlen(var);
    c->s = var;

    bc->variables = smm_realloc(bc->variables, sizeof(t_bytecode_variable *) * (bc->variables_len + 1));
    bc->variables[bc->variables_len] = c;
    bc->variables_len++;
}


static void _read_buffer(char *buf, int *bufptr, int size, void *data) {
    memcpy(data, buf+*bufptr, size);
    *bufptr += size;
}

static void _write_buffer(char *buf, int *bufptr, int size, void *data) {
    memcpy(buf+*bufptr, data, size);
    *bufptr += size;
}


/**
 * Convert binary stream to a bytecode structure (NOTE: bytecode is an unallocated pointer!)
 */
int convert_binary_to_bytecode(int bincode_len, char *bincode, t_bytecode *bytecode) {
    int pos = 0;

    bytecode = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
    bzero(bytecode, sizeof(bytecode));

    _read_buffer(bincode, &pos, sizeof(int), (void *)bytecode->stack_size);
    _read_buffer(bincode, &pos, sizeof(int), (void *)bytecode->code_len);
    _read_buffer(bincode, &pos, bytecode->code_len, (void *)bytecode->code);

    _read_buffer(bincode, &pos, sizeof(int), (void *)bytecode->constants_len);
    for (int i=0; i!=bytecode->constants_len; i++) {
        char type; int len; char *ptr;
        _read_buffer(bincode, &pos, sizeof(char), &type);
        _read_buffer(bincode, &pos, sizeof(int), &len);
        _read_buffer(bincode, &pos, len, ptr);
        _new_constant(bytecode, type, len, ptr);
    }

    _read_buffer(bincode, &pos, sizeof(int), (void *)bytecode->variables_len);
    for (int i=0; i!=bytecode->variables_len; i++) {
        int len; char *ptr;
        _read_buffer(bincode, &pos, sizeof(int), (void *)&len);
        _read_buffer(bincode, &pos, len, (void *)ptr);
        _new_variable(bytecode, ptr);
    }

    return 1;
}


/**
 * Convert bytecode structure into a binary stream (NOTE: bincode is an unallocated pointer!)
 */
int convert_bytecode_to_binary(t_bytecode *bytecode, int *bincode_len, char *bincode) {
    _write_buffer(bincode, bincode_len, sizeof(int), (void *)bytecode->stack_size);
    _write_buffer(bincode, bincode_len, sizeof(int), (void *)bytecode->code_len);
    _write_buffer(bincode, bincode_len, bytecode->code_len, (void *)bytecode->code);
    _write_buffer(bincode, bincode_len, sizeof(int), (void *)bytecode->constants_len);
    for (int i=0; i!=bytecode->constants_len; i++) {
        _write_buffer(bincode, bincode_len, sizeof(char), (void *)bytecode->constants[i]->type);
        _write_buffer(bincode, bincode_len, sizeof(int), (void *)bytecode->constants[i]->len);
        _write_buffer(bincode, bincode_len, bytecode->constants[i]->len, (void *)bytecode->constants[i]->data.ptr);
    }

    _write_buffer(bincode, bincode_len, sizeof(int), (void *)bytecode->variables_len);
    for (int i=0; i!=bytecode->variables_len; i++) {
        _write_buffer(bincode, bincode_len, sizeof(int), (void *)bytecode->variables[i]->len);
        _write_buffer(bincode, bincode_len, bytecode->variables[i]->len, (void *)bytecode->variables[i]->s);
    }

    return 1;
}


/**
 * Load a bytecode from disk, optionally verify signature
 */
t_bytecode *load_bytecode_from_disk(const char *filename, int verify_signature) {
    FILE *f = fopen(filename, "rb");

    t_bytecode_binary_header header;
    fread(&header, sizeof(header), 1, f);

    char *bincode = (char *)smm_malloc(header.bytecode_len);
    fseek(f, header.bytecode_offset, SEEK_SET);
    fread(bincode, header.bytecode_len, 1, f);

    if (verify_signature == 0 && header.signature_offset != 0) {
        saffire_warning("A signature is present, but verification is disabled");
    }

    if (header.signature_offset != 0 && verify_signature) {
        // Read signature
        char *signature = (char *)smm_malloc(header.signature_len);
        fseek(f, header.signature_offset, SEEK_SET);
        fread(signature, header.signature_len, 1, f);

        // @TODO: Verify!
        printf("@TODO: verify the signature!");
    }

    fclose(f);

    // Convert binary to bytecode
    t_bytecode *bc = NULL;
    if (! convert_binary_to_bytecode(header.bytecode_len, bincode, bc)) {
        saffire_error("Could not convert bytecode data");
    }

    // Return bytecode
    return bc;
}


/**
 * Save a bytecode from disk, optionally sign and add signature
 */
void save_bytecode_to_disk(const char *dest_filename, const char *source_filename, t_bytecode *bc, int sign) {
    // Convert
    int bincode_len;
    char *bincode = NULL;
    if (! convert_bytecode_to_binary(bc, &bincode_len, bincode)) {
        saffire_error("Could not convert bytecode data");
    }

    t_bytecode_binary_header header;
    header.magic = MAGIC_HEADER;

    // Fetch modification time from source file and fill into header
    struct stat sb;
    if (! stat(source_filename, &sb)) {
        header.timestamp = sb.st_mtime;
    } else {
        header.timestamp = 0;
    }

    // Create file
    FILE *f = fopen(dest_filename, "wb");

    // temporary write header
    fwrite("\0", 1, sizeof(header), f);

    // Write bytecode
    header.bytecode_len = bincode_len;
    header.bytecode_offset = ftell(f);
    fwrite(bincode, bincode_len, 1, f);

    // Add signature at the end of the file
    if (sign == 1) {
        // Create signature
        int gpg_signature_len = 0;
        char *gpg_signature = "blaat";

        header.signature_offset = ftell(f);
        header.signature_len = gpg_signature_len;
        fwrite(gpg_signature, gpg_signature_len, 1, f);
    }

    // Reset to the start of the file and write header
    fseek(f, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, f);

    fclose(f);
}



/**
 * Create dummy bytecode instance
 * @TODO remove me
 */
t_bytecode *generate_dummy_bytecode(void) {
    char dummy_code[] =
                        // a = 0x1234;
                        "\x81\x00\x00\x00\x00"      //    0  LOAD_CONST   0 (0x1234)
                        "\x80\x00\x00\x00\x00"      //    5  STORE_VAR    0 (a)
                        // b = 0x5678;
                        "\x81\x01\x00\x00\x00"      //   10  LOAD_CONST   1 (0x5678)
                        "\x80\x01\x00\x00\x00"      //   15  STORE_VAR    1 (b)
                        // a.print();
                        "\x82\x00\x00\x00\x00"      //   20  LOAD_VAR     0 (a)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"
                        // b.print();
                        "\x82\x01\x00\x00\x00"      //   20  LOAD_VAR     1 (b)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"
                        // (a, b) = (b, a)
                        "\x82\x00\x00\x00\x00"      //   36  LOAD_VAR     0 (a)
                        "\x82\x01\x00\x00\x00"      //   41  LOAD_VAR     1 (b)
                        "\x02"                      //   46  ROT_TWO
                        "\x80\x00\x00\x00\x00"      //   47  STORE_VAR    0 (a)
                        "\x80\x01\x00\x00\x00"      //   52  STORE_VAR    1 (b)
                        // a.print()
                        "\x82\x00\x00\x00\x00"      //   20  LOAD_VAR     0 (a)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"
                        // b.print()
                        "\x82\x01\x00\x00\x00"      //   20  LOAD_VAR     1 (b)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"

                        "\x00"                      //   69  STOP
                       ;

    t_bytecode *bc = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
    bc->stack_size = 10;
    bc->code_len = sizeof(dummy_code);
    bc->code = smm_malloc(bc->code_len);
    memcpy(bc->code, dummy_code, bc->code_len);

    bc->constants = NULL;
    bc->variables = NULL;

    // constants
    _new_constant(bc, BYTECODE_CONST_NUMERICAL, 4, (void *)0x1234);
    _new_constant(bc, BYTECODE_CONST_NUMERICAL, 4, (void *)0x5678);
    _new_constant(bc, BYTECODE_CONST_OBJECT, 4, "print");
    _new_variable(bc, "a");
    _new_variable(bc, "b");


    save_bytecode_to_disk("bytecode.sfc", "bytecode.sf", bc, 0);

    return bc;
}
