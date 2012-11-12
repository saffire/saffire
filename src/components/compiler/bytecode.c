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
#include <bzlib.h>
#include "compiler/bytecode.h"
#include "compiler/ast.h"
#include "general/dll.h"
#include "general/smm.h"
#include "version.h"
#include "vm/vm_opcodes.h"
#include "interpreter/errors.h"


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
static void _new_constant_string(t_bytecode *bc, char *s) {
    // Setup constant
    t_bytecode_constant *c = (t_bytecode_constant *)smm_malloc(sizeof(t_bytecode_constant));
    c->type = BYTECODE_CONST_STRING;
    c->len = strlen(s);
    c->data.s = s;  // @TODO: strdupped?

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
 * Add a new variable to the bytecode structure
 */
static void _new_variable(t_bytecode *bc, char *var) {
    // Setup variable
    t_bytecode_variable *c = smm_malloc(sizeof(t_bytecode_variable));
    c->len = strlen(var);
    c->s = var;  // @TODO: strdupped?

    // Add variable
    bc->variables = smm_realloc(bc->variables, sizeof(t_bytecode_variable *) * (bc->variables_len + 1));
    bc->variables[bc->variables_len] = c;
    bc->variables_len++;
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
 * Convert binary stream to a bytecode structure (NOTE: bytecode is an unallocated pointer!)
 */
t_bytecode *convert_binary_to_bytecode(int bincode_len, char *bincode) {
    int pos = 0;
    char *s; long l; int j;
    int clen, vlen;

    // Initialize new bytecode structure
    t_bytecode *bytecode = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
    bzero(bytecode, sizeof(bytecode));

    // Read headers
    _read_buffer(bincode, &pos, sizeof(uint32_t), &bytecode->stack_size);
    _read_buffer(bincode, &pos, sizeof(uint32_t), &bytecode->code_len);

    // Allocate memory for code and store
    bytecode->code = smm_malloc(bytecode->code_len);
    _read_buffer(bincode, &pos, bytecode->code_len, bytecode->code);

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
                _new_constant_string(bytecode, s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                _read_buffer(bincode, &pos, len, &l);
                _new_constant_long(bytecode, l);
                break;
            default :
                saffire_error("Unknown constant type %d\n", type);
                break;
        }

    }

    // Read all variables
    _read_buffer(bincode, &pos, sizeof(int), &vlen);
    for (int i=0; i!=vlen; i++) {
        _read_buffer(bincode, &pos, sizeof(int), &j);

        // Variable strings do not have a trailing \0 on disk.
        s = smm_malloc(j+1);
        _read_buffer(bincode, &pos, j, s);
        s[j] = '\0';
        _new_variable(bytecode, s);
    }

    return bytecode;
}


/**
 * Convert bytecode structure into a binary stream (NOTE: bincode is an unallocated pointer!)
 */
int convert_bytecode_to_binary(t_bytecode *bytecode, int *bincode_len, char **bincode) {

    // Write headers and codeblock
    _write_buffer(bincode, bincode_len, sizeof(int), &bytecode->stack_size);
    _write_buffer(bincode, bincode_len, sizeof(int), &bytecode->code_len);
    _write_buffer(bincode, bincode_len, bytecode->code_len, bytecode->code);
    _write_buffer(bincode, bincode_len, sizeof(int), &bytecode->constants_len);

    // Write constants
    for (int i=0; i!=bytecode->constants_len; i++) {
        _write_buffer(bincode, bincode_len, sizeof(char), &bytecode->constants[i]->type);
        _write_buffer(bincode, bincode_len, sizeof(int), &bytecode->constants[i]->len);
        switch (bytecode->constants[i]->type) {
            case BYTECODE_CONST_STRING :
                _write_buffer(bincode, bincode_len, bytecode->constants[i]->len, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                _write_buffer(bincode, bincode_len, bytecode->constants[i]->len, &bytecode->constants[i]->data.l);
                break;
            default :
                saffire_error("Unknown constant type %d\n", bytecode->constants[i]->type);
                break;
        }
    }

    // Write variables
    _write_buffer(bincode, bincode_len, sizeof(int), &bytecode->variables_len);
    for (int i=0; i!=bytecode->variables_len; i++) {
        _write_buffer(bincode, bincode_len, sizeof(int), &bytecode->variables[i]->len);
        _write_buffer(bincode, bincode_len, bytecode->variables[i]->len, bytecode->variables[i]->s);
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

    // Uncompress bincode block if needed
    if ((header.flags & BYTECODE_FLAG_COMPRESSED) == BYTECODE_FLAG_COMPRESSED) {
        // Allocate uncompressed size buffer based on info from the header
        char *bzipblock = smm_malloc(header.bytecode_uncompressed_len);
        unsigned int bzipblock_len;
        int ret = BZ2_bzBuffToBuffDecompress(bzipblock, &bzipblock_len, bincode, header.bytecode_len, 0, 0);
        if (ret != BZ_OK) {
            saffire_error("Error while compressing data.");
        }

        // Sanity check. These should match
        if (bzipblock_len != header.bytecode_uncompressed_len) {
            saffire_error("Header information does not match with the size of the uncompressed data block");
        }

        // Free unpacked binary code
        smm_free(bincode);

        // Set data to uncompressed block
        bincode = bzipblock;
        header.bytecode_len = bzipblock_len;
    }


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
    t_bytecode *bc = convert_binary_to_bytecode(header.bytecode_len, bincode);
    if (! bc) {
        saffire_error("Could not convert bytecode data");
    }

    // Return bytecode
    return bc;
}


/**
 * Save a bytecode from disk, optionally sign and add signature
 */
void save_bytecode_to_disk(const char *dest_filename, const char *source_filename, t_bytecode *bc, int sign, int compress) {
    // Convert
    int bincode_len = 0;
    char *bincode = NULL;
    if (! convert_bytecode_to_binary(bc, &bincode_len, &bincode)) {
        saffire_error("Could not convert bytecode data");
    }

    t_bytecode_binary_header header;
    bzero(&header, sizeof(header));
    header.magic = MAGIC_HEADER;

    // Fetch modification time from source file and fill into header
    struct stat sb;
    if (! stat(source_filename, &sb)) {
        header.timestamp = sb.st_mtime;
    } else {
        header.timestamp = 0;
    }

    // Set header flags
    header.flags = 0;
    if (sign) header.flags |= BYTECODE_FLAG_SIGNED;
    if (compress) header.flags |= BYTECODE_FLAG_COMPRESSED;

    // Save lengths of the bytecode
    header.bytecode_len = bincode_len;
    header.bytecode_uncompressed_len = bincode_len;


    // Need to compress bincode block?
    if (compress) {
        // http://www.bzip.org/1.0.5/bzip2-manual-1.0.5.html#hl-interface recommends 101% of uncompressed size + 600 bytes
        char *bzipblock = smm_malloc(bincode_len + (bincode_len / 100) + 600);
        unsigned int bzipblock_len;
        int ret = BZ2_bzBuffToBuffCompress(bzipblock, &bzipblock_len, bincode, bincode_len, 9, 0, 30);
        if (ret != BZ_OK) {
            saffire_error("Error while compressing data.");
        }

        // Forget about the original bincode and replace it with out bzip2 data
        smm_free(bincode);
        bincode = bzipblock;
        bincode_len = bzipblock_len;

        header.bytecode_len = bzipblock_len;
    }



    // Create file
    FILE *f = fopen(dest_filename, "wb");

    // temporary write header
    fwrite("\0", 1, sizeof(header), f);

    // Write bytecode
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

    // Free up our binary code
    smm_free(bincode);
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

    bc->constants = NULL;   // Important to start constants and variables on NULL
    bc->variables = NULL;

    // constants
    _new_constant_long(bc, 0x1234);
    _new_constant_long(bc, 0x5678);
    _new_constant_string(bc, "print");
    _new_variable(bc, "a");
    _new_variable(bc, "b");

    save_bytecode_to_disk("bytecode.sfc", "bytecode.sf", bc, 0, 1);


    t_bytecode *new_bc = load_bytecode_from_disk("bytecode.sfc", 0);
    return new_bc;
}
