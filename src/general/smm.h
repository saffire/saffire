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
#ifndef __SMM_H__
#define __SMM_H__

    #include <stdlib.h>

    // Maximum number of tags!
    #define SMM_MAX_TAGS     25

    #define SMM_TAG_INTERNAL   0                // Internal SMM allocations
    #define SMM_TAG_PLAIN      1                // Called through standard malloc/free/realloc methods
    #define SMM_TAG_HASH       2                // Hash tables
    #define SMM_TAG_SVAR       3                // Svars
    #define SMM_TAG_AST        4                // AST
    #define SMM_TAG_PARSER     5                // bison parser vars
    #define SMM_TAG_OTHER      SMM_MAX_TAGS-1   // Everything else that doesn't fit another category

    typedef struct smm_tag {
        int tag;                // Tag number
        int cur_element_count;  // Current number of elements
        int max_element_count;  // Maximum seen number of elements
        int cur_size;           // Current size in bytes
        int max_size;           // Maximum seen size in bytes
        int untracked_count;    // Number of untracked elements
        int untracked_size;     // Size in bytes of untracked data
    } t_smm_tag;

    void smm_init();

    void *smm_malloc(int tag, size_t size);
    void *smm_realloc(int tag, void *ptr, size_t size);
    void smm_free(int tag, void *ptr);
    t_smm_tag *smm_get_stats(int tag);

#endif