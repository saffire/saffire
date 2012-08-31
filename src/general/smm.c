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
#include <stdio.h>
#include <malloc.h>
#include "hashtable.h"
#include "smm.h"



/**
 *
 * Using malloc_hook to change malloc() free() and realloc(). Not that portable though, and
 * probably not that re-entrant either.
 *
 */

// Storage for old hooks.
static void *(*old_malloc_hook)(size_t, const void *);
static void  (*old_free_hook)(void *, const void *);
static void *(*old_realloc_hook)(void *, size_t, const void *);

/**
 * New malloc() call
 */
static void *new_malloc_hook(size_t size, const void *caller) {
    return smm_malloc(SMM_TAG_PLAIN, size);
}

/**
 * New free() call
 */
static void new_free_hook(void *ptr, const void *caller) {
    return smm_free(SMM_TAG_PLAIN, ptr);
}

static void *new_realloc_hook(void *ptr, size_t size, const void *caller) {
    return smm_realloc(SMM_TAG_PLAIN, ptr, size);
}


/**
 * Set the alloc hooks to point to our new functions (and save the old ones)
 */
static void init_alloc_hooks(void) {
    old_malloc_hook = __malloc_hook;
    __malloc_hook = new_malloc_hook;

    old_realloc_hook = __realloc_hook;
    __realloc_hook = new_realloc_hook;

    old_free_hook = __free_hook;
    __free_hook = new_free_hook;
}


/**
 * This is a nifty GLIBC hook that is called prior to calling main().
 */
void (*__malloc_initialize_hook)(void) = init_alloc_hooks;



/*
    The Saffire Memory Manager manages all memory for saffire. This includes constants, strings, objects etc, but
    also "internal" variables like linked lists, hash tables etc.

    All items that can be allocated are categorized into "tags". This allows us to actually see how memory is
    partitioned and used.

    Another thing that SMM does, is managing the memory usage. Note that the number of bytes SMM will be reporting will
    never be the actual bytes reserved by the OS. This is because systems like malloc() actually allocates a MINIMUM
    number of bytes. When allocating 11 bytes, we can only know for sure that a minimum of 11 bytes are allocated, but
    it's up to malloc() to decide how many bytes it actually reserved (could be 12 bytes to make sure things stays
    aligned, for instance.

    Now, tracking memory usage that is allocated with malloc is very hard. This is mostly because we do not know the
    size of a memory block after allocation. Somewhere, we must store this size, so whenever we realloc() or free() the
    memory, we know how much memory to decrease.

    Because storing this information inside a separate table will be very unoptimal, we actually store this information
    in front of the allocated block. This means, that whenever we want to allocate a 20 byte block, we are allocating a
    24byte (20+4 byte for the size). In the begin (offset 0), we are storing the size of the block 20, not 24). And we
    will be returning the pointer to offset 4, so other saffire components won't overwrite this information by accident.

    Now, this might work (and is portable enough), because now we can figure out how large a block is by fetching the
    size in front of the actual pointer. However, there are consequences: sometimes we NEED to make sure that blocks
    are aligned correctly. By adding a 4/8-byte (size_t) size, might mis-align this info.

    Therefor we also have a malloc_untracked and free_untracked version. Blocks created and freed do not have the size
    in front of the block. As a result, we cannot use these block to reallocate it's size, nor can we redecude the
    memory used when we free the block.

    This is why the smm_table structure has a untracked_size and untracked_count entry, which returns the number of
    allocated untracked blocks (the count is the actual count), but the size will be a max_size. This number will ALWAYS
    increase.

    Try to use untracked blocks as little as possible!
 */

// Table with all possible tags
t_smm_tag smm_tag_table[SMM_MAX_TAGS];


/**
 * Initialize tag table.
 *
 * There is no check if smm_init() has been called. We assume it is so everything has been
 * initialized properly.
 */
void smm_init() {
    // Initialize every tag
    for (int i=0; i!=SMM_MAX_TAGS; i++) {
        smm_tag_table[i].tag = i;
        smm_tag_table[i].cur_element_count = 0;
        smm_tag_table[i].max_element_count = 0;
        smm_tag_table[i].cur_size = 0;
        smm_tag_table[i].max_size = 0;
        smm_tag_table[i].untracked_count = 0;
        smm_tag_table[i].untracked_size = 0;
    }
}



/**
 *
 */
void smm_housekeeping_increase(int tag, int count, size_t size, int tracked) {
    // Do SMM housekeeping

    if (tracked) {
        smm_tag_table[tag].cur_element_count += count;
        if (smm_tag_table[tag].cur_element_count > smm_tag_table[tag].max_element_count) {
            smm_tag_table[tag].max_element_count = smm_tag_table[tag].cur_element_count;
        }

        smm_tag_table[tag].cur_size += size;
        if (smm_tag_table[tag].cur_size > smm_tag_table[tag].max_size) {
            smm_tag_table[tag].max_size = smm_tag_table[tag].cur_size;
        }
    } else {
        smm_tag_table[tag].untracked_count += count;
        smm_tag_table[tag].untracked_size += size;
    }
}

/**
 * Allocate memory. Accounting will be done in the current "tag". We track the size because the actual size is stored
 * in front of the actual block. This has some consequences: if we NEED alignment, we MUST use smm_malloc_untracked and
 * free with smm_free_untracked(). Never release memory with smm_free_untracked, that is actually allocated with
 * smm_malloc_tracked(). Bad things will happen as there are no checks.
 */
void *smm_malloc(int tag, size_t size) {
    if (tag >= SMM_MAX_TAGS) return NULL;

    // Increase size a bit so we can add the actually size of the block in front.
    size = size + sizeof(size_t);

    smm_housekeeping_increase(tag, 1, size, 1);

    // Switch the hook, call the (actual) malloc() and switch back
    __malloc_hook = old_malloc_hook;
    void *ptr = malloc(size);
    __malloc_hook = new_malloc_hook;
    if (ptr == NULL) {
        printf("Unable to allocate any more memory.\n");
        exit(1);
    }

    // Set size of this block in the first size_t element
    ((size_t *)ptr)[0] = size;

    // return a pointer AFTER the first size_t
    return (void *)&((size_t *)ptr)[1];
}


/**
 *
 */
void *smm_realloc(int tag, void *ptr, size_t size) {
    if (tag >= SMM_MAX_TAGS) return NULL;

    // Increase size a bit so we can add the actually size of the block in front.
    size = size + sizeof(size_t);

    // Do correct housekeeping!
    ptr = (void *) (((size_t *) ptr) - 1);
    size_t old_size = ((size_t *)ptr)[0];

    smm_tag_table[tag].cur_size -= old_size;
    smm_tag_table[tag].cur_size += size;

    // Switch the hook, call the (actual) realloc() and switch back
    __realloc_hook = old_realloc_hook;
    void *dst_ptr = realloc(ptr, size);
    __realloc_hook = new_realloc_hook;

    return (void *)&((size_t *)dst_ptr)[1];
}




/**
 *
 */
void smm_free(int tag, void *ptr) {
    // Get the size that is stored in front of the pointer
    ptr = (void *) (((size_t *) ptr) - 1);
    size_t size = ((size_t *)ptr)[0];

    // Decrease tag table info
    smm_tag_table[tag].cur_size -= size;
    smm_tag_table[tag].cur_element_count--;

    __free_hook = old_free_hook;
    free(ptr);
    __free_hook = new_free_hook;
}


/**
 * Allocate untracked memory. Note that this memory is not released, but counted separately
 */
void *smm_malloc_untracked(int tag, size_t size) {
    if (tag >= SMM_MAX_TAGS) return NULL;

    smm_housekeeping_increase(tag, 1, size, 0);

    // Switch the hook, call the (actual) malloc() and switch back
    __malloc_hook = old_malloc_hook;
    void *ptr = malloc(size);
    __malloc_hook = new_malloc_hook;

    if (ptr == NULL) {
        printf("Unable to allocate any more memory.\n");
        exit(1);
    }

    return ptr;
}


/**
 *
 */
void smm_free_untracked(int tag, void *ptr) {
    // Decrease tag table info
    smm_tag_table[tag].untracked_count--;

    // Free pointer
    free(ptr);
}


/**
 *
 */
t_smm_tag *smm_get_stats(int tag) {
    if (tag >= SMM_MAX_TAGS) return NULL;

    for (int i=0; i!=SMM_MAX_TAGS; i++) {
        if (smm_tag_table[i].max_element_count == 0) continue;
        printf("Tag       : %d\n", smm_tag_table[i].tag);
        printf("\tCur El Cnt  : %d\n", smm_tag_table[i].cur_element_count);
        printf("\tMax El Cnt  : %d\n", smm_tag_table[i].max_element_count);
        printf("\tCur Size    : %d\n", smm_tag_table[i].cur_size);
        printf("\tMax Size    : %d\n", smm_tag_table[i].max_size);
        printf("\tUntrack Cnt : %d\n", smm_tag_table[i].untracked_count);
        printf("\tUntrack Size: %d\n", smm_tag_table[i].untracked_size);
    }

    return &smm_tag_table[tag];
}
