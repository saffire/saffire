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
#include <stdio.h>
#include "general/dll.h"
#include "general/smm.h"


/**
 * Initialize an empty DLL
 */
t_dll *dll_init(void) {
    t_dll *dll = (t_dll *)smm_malloc(sizeof(t_dll));

    dll->size = 0;
    dll->head = NULL;
    dll->tail = NULL;
    return dll;
}

/**
 * Free a DLL (assumes data in elements are already freed)
 */
void dll_free(t_dll *dll) {
    t_dll_element *cur = dll->head;

    while (cur != NULL) {
        t_dll_element *e = cur;
        cur = cur->next;
        smm_free(e);
    }
    smm_free(dll);
}


/**
 * Prepend an element before the head of the DLL.
 */
t_dll_element *dll_prepend(t_dll *dll, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // Set head element data
    e->data = data;
    e->prev = NULL;
    e->next = dll->head;

    // First element, this becomes the tail as well
    if (dll->tail == NULL) {
        dll->tail = e;
    }

    // Already an head element present, make sure previous link is corrected
    if (dll->head != NULL) {
        dll->head->prev = e;
    }

    // This element becomes the head
    dll->head = e;

    // Increase DLL size
    dll->size++;

    return e;
}

/**
 * Append an element after the tail of the DLL.
 */
t_dll_element *dll_append(t_dll *dll, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // Set tail element data
    e->data = data;
    e->prev = dll->tail;
    e->next = NULL;

    // First element, this becomes the head as well
    if (dll->head == NULL) {
        dll->head = e;
    }

    // Already an tail element present, make sure next link is corrected
    if (dll->tail != NULL) {
        dll->tail->next = e;
    }

    // This element becomes the tail
    dll->tail = e;

    // Increase DLL size
    dll->size++;

    return e;
}


/**
 * Insert an element before the specified element. There is no check if the specified
 * element is already present in the DLL.
 */
t_dll_element *dll_insert_before(t_dll *dll, t_dll_element *before_element, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // No element given, just prepend
    if (before_element == NULL) {
        return dll_prepend(dll, data);
    }

    e->data = data;
    e->prev = before_element->prev;
    e->next = before_element;

    if (before_element->prev) {
        before_element->prev->next = e;
    }
    before_element->prev = e;


    if (before_element == dll->head) {
        dll->head = e;
    }

    dll->size++;

    return e;
}

/**
 * Insert an element after the specified element. There is no check if the specified
 * element is already present in the DLL.
 */
t_dll_element *dll_insert_after(t_dll *dll, t_dll_element *after_element, void *data) {
    t_dll_element *e = (t_dll_element *)smm_malloc(sizeof(t_dll_element));

    // No element given, just append
    if (after_element == NULL) {
        return dll_append(dll, data);
    }

    e->data = data;
    e->prev = after_element;
    e->next = after_element->next;

    if (after_element->next) {
        after_element->next->prev = e;
    }
    after_element->next = e;

    if (after_element == dll->tail) {
        dll->tail = e;
    }

    dll->size++;

    return e;
}


/**
 * Remove an element from the linked list
 */
int dll_remove(t_dll *dll, t_dll_element *element) {
    dll->size--;

    if (dll->size == 0) {
        dll->head = NULL;
        dll->tail = NULL;
    } else if (element == dll->head) {
        dll->head = dll->head->next;
        dll->head->prev = NULL;
        return 1;
    } else if (element == dll->tail) {
        dll->tail = dll->tail->prev;
        dll->tail->next = NULL;
        return 1;
    } else {
        element->prev->next = element->next;
        element->next->prev = element->prev;
    }

    return 1;
}

/**
 * Pushes data at the tail of the DLL
 */
void dll_push(t_dll *dll, void *data) {
    dll_insert_after(dll, DLL_TAIL(dll), data);
}

/**
 * Pops data at the tail of the DLL
 */
void *dll_pop(t_dll *dll) {
    t_dll_element *e = DLL_TAIL(dll);
    if (!e) return NULL;

    void *ret = e->data;
    dll_remove(dll, e);
    return ret;
}

/*
 * Peeks at data at the tail of the DLL
 */
void *dll_top(t_dll *dll) {
    t_dll_element *e = DLL_TAIL(dll);
    if (!e) return NULL;

    return e->data;
}

/**
 * Searches for specfici offset
 */
t_dll_element *dll_seek_offset(t_dll *dll, int offset) {
    if (offset < 0 || offset >= dll->size) return NULL;

    t_dll_element *e = DLL_HEAD(dll);
    while (offset) {
        offset--;
        e = DLL_NEXT(e);
    }

    return e;
}
