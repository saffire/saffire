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
#include "compiler/bytecode.h"
#include "general/smm.h"
#include "general/gpg.h"
#include "general/bzip2.h"
#include "general/config.h"

/**
 *
 */
static void saffire_compile_warning(char *str, ...) {
    va_list args;
    va_start(args, str);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, str, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/**
 *
 */
static void saffire_compile_error(char *str, ...) {
    va_list args;
    va_start(args, str);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, str, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
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
 * Add a new identifier to the bytecode structure
 */
static void _new_name(t_bytecode *bc, char *var) {
    // Setup identifier
    t_bytecode_identifier *c = smm_malloc(sizeof(t_bytecode_identifier));
    c->len = strlen(var);
    c->s = var;  // @TODO: strdupped?

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
 * Converts a binary stream to a bytecode structure (NOTE: bytecode must be an unallocated pointer!)
 */
static t_bytecode *bytecode_bc2bin(char *bincode) {
    int pos = 0;
    char *s; long l; int j;
    int clen, vlen;

    // Initialize new bytecode structure
    t_bytecode *bytecode = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
    bzero(bytecode, sizeof(t_bytecode));

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
                saffire_compile_error("Unknown constant type %d\n", type);
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
    }

    return bytecode;
}


/**
 * Convert bytecode structure into a binary stream (NOTE: bincode is an unallocated pointer!)
 */
static int bytecode_bin2bc(t_bytecode *bytecode, int *bincode_off, char **bincode) {

    // Write headers and codeblock
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->stack_size);
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->code_len);
    _write_buffer(bincode, bincode_off, bytecode->code_len, bytecode->code);
    _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants_len);

    // Write constants
    for (int i=0; i!=bytecode->constants_len; i++) {
        _write_buffer(bincode, bincode_off, sizeof(char), &bytecode->constants[i]->type);
        _write_buffer(bincode, bincode_off, sizeof(int), &bytecode->constants[i]->len);
        switch (bytecode->constants[i]->type) {
            case BYTECODE_CONST_STRING :
                _write_buffer(bincode, bincode_off, bytecode->constants[i]->len, bytecode->constants[i]->data.s);
                break;
            case BYTECODE_CONST_NUMERICAL :
                _write_buffer(bincode, bincode_off, bytecode->constants[i]->len, &bytecode->constants[i]->data.l);
                break;
            default :
                saffire_compile_error("Unknown constant type %d\n", bytecode->constants[i]->type);
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

    if (! bytecode_is_valid_file(filename)) {
        return NULL;
    }

    // Read header
    FILE *f = fopen(filename, "rb");
    fread(&header, sizeof(header), 1, f);

    // Allocate room and read binary code
    char *bincode = (char *)smm_malloc(header.bytecode_len);
    fseek(f, header.bytecode_offset, SEEK_SET);
    fread(bincode, header.bytecode_len, 1, f);


    // There is a signature present. Give warning when the user does not want to check it
    if (verify_signature == 0 &&
        (header.flags & BYTECODE_FLAG_SIGNED) == BYTECODE_FLAG_SIGNED &&
        header.signature_offset != 0) {
        saffire_compile_warning("A signature is present, but verification is disabled");
    }

    // We need to check signature, and there is one present
    if (verify_signature == 1 &&
        (header.flags & BYTECODE_FLAG_SIGNED) == BYTECODE_FLAG_SIGNED &&
        header.signature_offset != 0) {

        // Read signature
        char *signature = (char *)smm_malloc(header.signature_len);
        fseek(f, header.signature_offset, SEEK_SET);
        fread(signature, header.signature_len, 1, f);

        // Verify signature
        if (! gpg_verify(bincode, header.bytecode_len, signature, header.signature_len)) {
            saffire_compile_error("The signature for this bytecode is INVALID!");
        }
    }

    fclose(f);

    // Uncompress bincode block
    unsigned int bzip_buf_len = header.bytecode_uncompressed_len;
    char *bzip_buf = smm_malloc(bzip_buf_len);
    if (! bzip2_decompress(bzip_buf, &bzip_buf_len, bincode, header.bytecode_len)) {
        saffire_compile_error("Error while decompressing data");
    }

    // Sanity check. These should match
    if (bzip_buf_len != header.bytecode_uncompressed_len) {
        saffire_compile_error("Header information does not match with the size of the uncompressed data block");
    }

    // Free unpacked binary code. We don't need it anymore
    smm_free(bincode);

    // Set bincode data to the uncompressed block
    bincode = bzip_buf;
    header.bytecode_len = bzip_buf_len;

    // Convert binary to bytecode
    t_bytecode *bc = bytecode_bc2bin(bincode);
    if (! bc) {
        saffire_compile_error("Could not convert bytecode data");
    }

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
    if (! bytecode_bin2bc(bc, &bincode_len, &bincode)) {
        saffire_compile_error("Could not convert bytecode data");
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
        printf("TIMESTAMP: %d\n", header.timestamp);
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
        saffire_compile_error("Error while compressing data");
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
    // @TODO: Implement this
}


/**
 * Create dummy bytecode instance
 * @TODO remove me
 */
t_bytecode *generate_dummy_bytecode(void) {
    char dummy_code[] =
//                        // import io as io from io; (or: import io)
//                        "\x81\x03\x00\x00\x00"      //    LOAD_CONST     3 (io)
//                        "\x81\x03\x00\x00\x00"      //    LOAD_CONST     3 (io)
//                        "\x7F"                      //    IMPORT
//                        "\x80\x03\x00\x00\x00"      //    STORE_ID       3 (io)
//
//
//                        // import console as the_con, io as io from io;
//                        "\x81\x04\x00\x00\x00"      //    LOAD_CONST     4 (console)
//                        "\x81\x03\x00\x00\x00"      //    LOAD_CONST     3 (io)
//                        "\x80\x04\x00\x00\x00"      //    STORE_ID       4 (the_con)
//
//                        "\x81\x03\x00\x00\x00"      //    LOAD_CONST     3 (io)
//                        "\x81\x03\x00\x00\x00"      //    LOAD_CONST     3 (io)
//                        "\x80\x04\x00\x00\x00"      //    STORE_ID       4 (the_con)
//                        "\x7F"                      //    IMPORT

                        // a = 0x1234;
                        "\x81\x00\x00\x00\x00"      //    0  LOAD_CONST    0 (0x1234)
                        "\x80\x00\x00\x00\x00"      //    5  STORE_ID      0 (a)
                        // b = 0x5678;
                        "\x81\x01\x00\x00\x00"      //   10  LOAD_CONST   1 (0x5678)
                        "\x80\x01\x00\x00\x00"      //   15  STORE_ID     1 (b)

                        // c = a + b;
                        "\x82\x00\x00\x00\x00"      //   20  LOAD_ID       0 (a)
                        "\x82\x01\x00\x00\x00"      //   20  LOAD_ID       0 (b)
                        "\x17"                      //   20  BINARY_ADD
                        "\x80\x02\x00\x00\x00"      //   15  STORE_ID     1 (c)

                        // c = c + c;
                        "\x82\x02\x00\x00\x00"      //   20  LOAD_ID       0 (c)
                        "\x82\x02\x00\x00\x00"      //   20  LOAD_ID       0 (c)
                        "\x17"                      //   20  BINARY_ADD
                        "\x80\x02\x00\x00\x00"      //   15  STORE_ID     1 (c)


                        // c.print();
                        "\x82\x02\x00\x00\x00"      //   20  LOAD_ID       0 (c)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"


                        "\x82\x05\x00\x00\x00"      //       LOAD_ID      4 (::_sfl::io)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"

                        "\x00"                      //   69  STOP

                        // a.print();
                        "\x82\x00\x00\x00\x00"      //   20  LOAD_ID       0 (a)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"

                        "\x83\x0E\x00\x00\x00"      //   JMP FORWARD

                        // b.print();
                        "\x82\x01\x00\x00\x00"      //   20  LOAD_ID       1 (b)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"


                        "\x82\x00\x00\x00\x00"      //   20  LOAD_ID       0 (a)
                        "\x84\x0E\x00\x00\x00"      //   JUMP_IF_TRUE  1:

                        // b.print();
                        "\x82\x01\x00\x00\x00"      //   20  LOAD_ID       1 (b)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"
                        // 1:
                        // a.print();
                        "\x82\x00\x00\x00\x00"      //   20  LOAD_ID       0 (a)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"

                        "\x00"                      //   69  STOP


                        // (a, b) = (b, a)
                        "\x82\x00\x00\x00\x00"      //   36  LOAD_ID       0 (a)
                        "\x82\x01\x00\x00\x00"      //   41  LOAD_ID       1 (b)
                        "\x02"                      //   46  ROT_TWO
                        "\x80\x00\x00\x00\x00"      //   47  STORE_ID     0 (a)
                        "\x80\x01\x00\x00\x00"      //   52  STORE_ID     1 (b)
                        // a.print()
                        "\x82\x00\x00\x00\x00"      //   20  LOAD_ID       0 (a)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"
                        // b.print()
                        "\x82\x01\x00\x00\x00"      //   20  LOAD_ID       1 (b)
                        "\xC0\x02\x00\x00\x00"      //   25  CALL_METHOD  2 (print), 0
                            "\x00\x00\x00\x00"

                        "\x00"                      //   69  STOP
                       ;

    t_bytecode *bc = (t_bytecode *)smm_malloc(sizeof(t_bytecode));
    bzero(bc, sizeof(t_bytecode));
    bc->stack_size = 10;
    bc->code_len = sizeof(dummy_code);
    bc->code = smm_malloc(bc->code_len);
    memcpy(bc->code, dummy_code, bc->code_len);

    bc->constants = NULL;   // Important to start constants and identifiers on NULL
    bc->identifiers = NULL;

    // constants
    _new_constant_long(bc, 0x1234);
    _new_constant_long(bc, 0x5678);
    _new_constant_string(bc, "print");
    _new_constant_string(bc, "io");
    _new_constant_string(bc, "console");
    _new_name(bc, "a");
    _new_name(bc, "b");
    _new_name(bc, "c");
    _new_name(bc, "io");
    _new_name(bc, "the_con");
    _new_name(bc, "::_sfl::io");

    return bc;
}


/**
 *
 */
int bytecode_get_timestamp(const char *path) {
    t_bytecode_binary_header header;

    // Read header
    FILE *f = fopen(path, "rb");
    fread(&header, sizeof(header), 1, f);
    fclose(f);

    return header.timestamp;
}

/**
 *
 */
int bytecode_is_valid_file(const char *path) {
    t_bytecode_binary_header header;

    // Read header
    FILE *f = fopen(path, "rb");
    fread(&header, sizeof(header), 1, f);
    fclose(f);

    return (header.magic == MAGIC_HEADER);
}


/**
 *
 */
int bytecode_is_signed(const char *path) {
    t_bytecode_binary_header header;

    // Read header
    FILE *f = fopen(path, "rb");
    fread(&header, sizeof(header), 1, f);
    fclose(f);

    return ((header.flags & BYTECODE_FLAG_SIGNED) == BYTECODE_FLAG_SIGNED &&
            header.signature_offset != 0);
}


/**
 *
 */
int bytecode_remove_signature(const char *path) {
    t_bytecode_binary_header header;

    // Sanity check
    if (! bytecode_is_signed(path)) return 1;

    // Read header
    FILE *f = fopen(path, "r+b");
    fread(&header, sizeof(header), 1, f);

    int sigpos = header.signature_offset;
    header.signature_offset = 0;
    header.signature_len = 0;
    header.flags &= ~BYTECODE_FLAG_SIGNED;

    // Write new header
    fseek(f, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, f);

    // Strip away the signature (@TODO: assume signature is at end of file)
    ftruncate(fileno(f), sigpos);
    fclose(f);

    return 0;
}


/**
 * Add a new signature to the
 */
int bytecode_add_signature(const char *path, char *gpg_key) {
    t_bytecode_binary_header header;

    // Sanity check
    if (bytecode_is_signed(path)) return 1;

    // Read header
    FILE *f = fopen(path, "r+b");
    fread(&header, sizeof(header), 1, f);

    // Allocate room and read bincode from file
    char *bincode = smm_malloc(header.bytecode_len);
    fseek(f, header.bytecode_offset, SEEK_SET);
    fread(bincode, header.bytecode_len, 1, f);

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
        printf("Cannot find GPG key. Please set the correct GPG key inside your INI file");
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
    fwrite(&header, sizeof(header), 1, f);

    // Write signature to the end of the file (signature offset)
    fseek(f, header.signature_offset, SEEK_SET);
    fwrite(gpg_signature, gpg_signature_len, 1, f);

    fclose(f);

    return 0;
}


