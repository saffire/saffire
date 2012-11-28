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
#include <stdarg.h>
#include <unistd.h>
#include "general/output.h"
#include "compiler/bytecode.h"
#include "general/smm.h"
#include "general/gpg.h"
#include "general/bzip2.h"
#include "general/config.h"

/**
 * Add constant to a bytecode structure
 */
static void _add_constant(t_bytecode *bc, t_bytecode_constant *c) {
    bc->constants = smm_realloc(bc->constants, sizeof(t_bytecode_constant *) * (bc->constants_len + 1));
    bc->constants[bc->constants_len] = c;
    bc->constants_len++;
}


static void _free_constant(t_bytecode_constant *c) {
     switch (c->type) {
         case BYTECODE_CONST_STRING :
            smm_free(c->data.s);
            break;
         case BYTECODE_CONST_NUMERICAL :
             break;
         case BYTECODE_CONST_CODE :
             bytecode_free(c->data.code);
             break;
         default :
             error_and_die(1, "Unknown constant type %d\n", c->type);
             break;
     }
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
    c->s = smm_strdup(var);

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
static t_bytecode *bytecode_bin2bc(char *bincode) {
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
            case BYTECODE_CONST_CODE :
                // Read binary buffer to new bytecode
                child_bytecode = bytecode_bin2bc(bincode+pos);
                pos += len; // Skip the just read binary bytecode
                _new_constant_code(bytecode, child_bytecode);
                break;
            default :
                error_and_die(1, "Unknown constant type %d\n", type);
                break;
        }
    }

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

    return bytecode;
}


/**
 * Convert bytecode structure into a binary stream (NOTE: bincode is an unallocated pointer!)
 */
static int bytecode_bc2bin(t_bytecode *bytecode, int *bincode_off, char **bincode) {
    char *child_bincode = NULL;
    int child_bincode_len = 0;

    // Write headers and codeblock
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->stack_size);
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->code_len);
    _write_buffer(bincode, bincode_off, bytecode->code_len, bytecode->code);
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants_len);

    // Write constants
    for (int i=0; i!=bytecode->constants_len; i++) {
        _write_buffer(bincode, bincode_off, sizeof(char), &bytecode->constants[i]->type);
        switch (bytecode->constants[i]->type) {
            case BYTECODE_CONST_CODE :
                child_bincode_len = 0;
                bytecode_bc2bin(bytecode->constants[i]->data.code, &child_bincode_len, &child_bincode);
                _write_buffer(bincode, bincode_off, sizeof(int), &child_bincode_len);
                _write_buffer(bincode, bincode_off, child_bincode_len, child_bincode);
                break;
            case BYTECODE_CONST_STRING :
                _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants[i]->len);
                _write_buffer(bincode, bincode_off, bytecode->constants[i]->len, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants[i]->len);
                _write_buffer(bincode, bincode_off, bytecode->constants[i]->len, &bytecode->constants[i]->data.l);
                break;
            default :
                error_and_die(1, "Unknown constant type %d\n", bytecode->constants[i]->type);
                break;
        }
    }

    // Write identifiers
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->identifiers_len);
    for (int i=0; i!=bytecode->identifiers_len; i++) {
        _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->identifiers[i]->len);
        _write_buffer(bincode, bincode_off, bytecode->identifiers[i]->len, bytecode->identifiers[i]->s);
    }

    return 1;
}


/**
 * Load a bytecode from disk, optionally verify signature
 */
t_bytecode *bytecode_load(const char *filename, int verify_signature) {
    t_bytecode_binary_header header;
    int unused; // @TODO: Make sure we actually use these values

    if (! bytecode_is_valid_file(filename)) {
        return NULL;
    }

    // Read header
    FILE *f = fopen(filename, "rb");
    unused = fread(&header, sizeof(header), 1, f);

    // Allocate room and read binary code
    char *bincode = (char *)smm_malloc(header.bytecode_len);
    fseek(f, header.bytecode_offset, SEEK_SET);
    unused = fread(bincode, header.bytecode_len, 1, f);


    // There is a signature present. Give warning when the user does not want to check it
    if (verify_signature == 0 &&
        (header.flags & BYTECODE_FLAG_SIGNED) == BYTECODE_FLAG_SIGNED &&
        header.signature_offset != 0) {
        output("A signature is present, but verification is disabled");
    }

    // We need to check signature, and there is one present
    if (verify_signature == 1 &&
        (header.flags & BYTECODE_FLAG_SIGNED) == BYTECODE_FLAG_SIGNED &&
        header.signature_offset != 0) {

        // Read signature
        char *signature = (char *)smm_malloc(header.signature_len);
        fseek(f, header.signature_offset, SEEK_SET);
        unused = fread(signature, header.signature_len, 1, f);

        // Verify signature
        if (! gpg_verify(bincode, header.bytecode_len, signature, header.signature_len)) {
            error_and_die(1, "The signature for this bytecode is INVALID!");
        }
    }

    fclose(f);

    // Uncompress bincode block
    unsigned int bzip_buf_len = header.bytecode_uncompressed_len;
    char *bzip_buf = smm_malloc(bzip_buf_len);
    if (! bzip2_decompress(bzip_buf, &bzip_buf_len, bincode, header.bytecode_len)) {
        error_and_die(1, "Error while decompressing data");
    }

    // Sanity check. These should match
    if (bzip_buf_len != header.bytecode_uncompressed_len) {
        error_and_die(1, "Header information does not match with the size of the uncompressed data block");
    }

    // Free unpacked binary code. We don't need it anymore
    smm_free(bincode);

    // Set bincode data to the uncompressed block
    bincode = bzip_buf;
    header.bytecode_len = bzip_buf_len;

    // Convert binary to bytecode
    t_bytecode *bc = bytecode_bin2bc(bincode);
    if (! bc) {
        error_and_die(1, "Could not convert bytecode data");
    }

    smm_free(bzip_buf);

    // Return bytecode
    return bc;
}


/**
 * Save a bytecode from disk, optionally sign and add signature
 */
void bytecode_save(const char *dest_filename, const char *source_filename, t_bytecode *bc) {
    char *bincode = NULL;
    int bincode_len = 0;

    // Convert bytecode to bincode
    if (! bytecode_bc2bin(bc, &bincode_len, &bincode)) {
        error_and_die(1, "Could not convert bytecode data");
    }

    // Let header point to the reserved header position
    t_bytecode_binary_header header;
    bzero(&header, sizeof(t_bytecode_binary_header));

    // Set header fields
    header.magic = MAGIC_HEADER;
    header.flags = 0;

    // Fetch modification time from source file and fill into header
    struct stat sb;
    if (! stat(source_filename, &sb)) {
        header.timestamp = sb.st_mtime;
    } else {
        header.timestamp = 0;
    }

    // Save lengths of the bytecode (assume we save it uncompressed for now)
    header.bytecode_uncompressed_len = bincode_len;
    header.bytecode_len = bincode_len;


    // Compress the bincode block
    // Compress buffer
    unsigned int bzip_buf_len = 0;
    char *bzip_buf = NULL;
    if (! bzip2_compress(&bzip_buf, &bzip_buf_len, bincode, bincode_len)) {
        error_and_die(1, "Error while compressing data");
    }

    // Forget about the original bincode and replace it with out bzip2 data.
    smm_free(bincode);
    bincode = bzip_buf;
    bincode_len = bzip_buf_len;

    // The actual bytecode binary length will differ from it's uncompressed length.
    header.bytecode_len = bzip_buf_len;

    // Create file
    FILE *f = fopen(dest_filename, "wb");

    // temporary write header
    fwrite("\0", 1, sizeof(header), f);

    // Write bytecode
    header.bytecode_offset = ftell(f);
    fwrite(bincode, bincode_len, 1, f);

    // Reset to the start of the file and write header
    fseek(f, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, f);

    fclose(f);

    // Free up our binary code
    smm_free(bincode);
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

    smm_free(bc);
}



/**
 *
 */
int bytecode_get_timestamp(const char *path) {
    t_bytecode_binary_header header;
    int unused;

    // Read header
    FILE *f = fopen(path, "rb");
    unused = fread(&header, sizeof(header), 1, f);
    fclose(f);

    return header.timestamp;
}

/**
 *
 */
int bytecode_is_valid_file(const char *path) {
    t_bytecode_binary_header header;
    int unused;

    // Read header
    FILE *f = fopen(path, "rb");
    unused = fread(&header, sizeof(header), 1, f);
    fclose(f);

    return (header.magic == MAGIC_HEADER);
}


/**
 *
 */
int bytecode_is_signed(const char *path) {
    t_bytecode_binary_header header;
    int unused;

    // Read header
    FILE *f = fopen(path, "rb");
    unused = fread(&header, sizeof(header), 1, f);
    fclose(f);

    return ((header.flags & BYTECODE_FLAG_SIGNED) == BYTECODE_FLAG_SIGNED &&
            header.signature_offset != 0);
}


/**
 *
 */
int bytecode_remove_signature(const char *path) {
    t_bytecode_binary_header header;
    int unused;

    // Sanity check
    if (! bytecode_is_signed(path)) return 1;

    // Read header
    FILE *f = fopen(path, "r+b");
    unused = fread(&header, sizeof(header), 1, f);

    int sigpos = header.signature_offset;
    header.signature_offset = 0;
    header.signature_len = 0;
    header.flags &= ~BYTECODE_FLAG_SIGNED;

    // Write new header
    fseek(f, 0, SEEK_SET);
    unused = fwrite(&header, sizeof(header), 1, f);

    // Strip away the signature (@TODO: assume signature is at end of file)
    unused = ftruncate(fileno(f), sigpos);
    fclose(f);

    return 0;
}


/**
 * Add a new signature to the
 */
int bytecode_add_signature(const char *path, char *gpg_key) {
    t_bytecode_binary_header header;
    int unused;

    // Sanity check
    if (bytecode_is_signed(path)) return 1;

    // Read header
    FILE *f = fopen(path, "r+b");
    unused = fread(&header, sizeof(header), 1, f);

    // Allocate room and read bincode from file
    char *bincode = smm_malloc(header.bytecode_len);
    fseek(f, header.bytecode_offset, SEEK_SET);
    unused = fread(bincode, header.bytecode_len, 1, f);

    // Create signature from bincode
    char *gpg_signature = NULL;
    unsigned int gpg_signature_len = 0;
    char *_gpg_key;
    if (gpg_key == NULL) {
        _gpg_key = config_get_string("gpg.key", NULL);
    } else {
        _gpg_key = gpg_key;
    }
    if (_gpg_key == NULL) {
        error("Cannot find GPG key. Please set the correct GPG key inside your INI file");
        return 1;
    }
    gpg_sign(_gpg_key, bincode, header.bytecode_len, &gpg_signature, &gpg_signature_len);

    // Set new header values
    fseek(f, 0, SEEK_END);
    header.signature_offset = ftell(f);
    header.signature_len = gpg_signature_len;
    header.flags |= BYTECODE_FLAG_SIGNED;

    // Write new header
    fseek(f, 0, SEEK_SET);
    unused = fwrite(&header, sizeof(header), 1, f);

    // Write signature to the end of the file (signature offset)
    fseek(f, header.signature_offset, SEEK_SET);
    unused = fwrite(gpg_signature, gpg_signature_len, 1, f);

    fclose(f);

    return 0;
}


/**
 * @TODO: Temporary bytecode includes
 */
#include "../../src/components/vm/bc001_bcs.c"
#include "../../src/components/vm/bc002_bcs.c"
#include "../../src/components/vm/bc003_bcs.c"
#include "../../src/components/vm/bc004_bcs.c"
#include "../../src/components/vm/bc005_bcs.c"


