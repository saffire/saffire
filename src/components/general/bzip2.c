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
#include "general/smm.h"
#include <bzlib.h>


#define BZIP_BLOCKSIZE               9
#define BZIP_WORK_FACTOR            30


/**
 * Compresses source into dest. Dest is newly allocated and it's length recorded inside dest_len
 */
int bzip2_compress(char **dest, unsigned int *dest_len, const char *source, unsigned int source_len) {
    /*
     * http://www.bzip.org/1.0.5/bzip2-manual-1.0.5.html#hl-interface recommends 101% of uncompressed size + 600 bytes
     */
    *dest_len = (source_len * 1.1) + 600;
    *dest = smm_malloc(*dest_len);

    int ret = BZ2_bzBuffToBuffCompress(*dest, dest_len, (char *)source, source_len, BZIP_BLOCKSIZE, 0, BZIP_WORK_FACTOR);
    return (ret == BZ_OK);
}


/**
 * Decompresses source into dest. Dest should be allocated and dest_len the length of that buffer.
 */
int bzip2_decompress(char *dest, unsigned int *dest_len, const char *source, unsigned int source_len) {
    // Decompress (slowly)
    int ret = BZ2_bzBuffToBuffDecompress(dest, dest_len, (char *)source, source_len, 0, 0);
    return (ret == BZ_OK);
}
