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
#ifndef __DLL_H__
#define __DLL_H__

    typedef struct _dll_element {
        struct _dll_element *prev;  // Pointer to previous element (or NULL)
        struct _dll_element *next;  // Pointer to next element (or NULL)
        void *data;                 // data container
    } t_dll_element;

    typedef struct _dll {
        long size;                  // Size of the DLL
        t_dll_element *head;        // Start "head" element
        t_dll_element *tail;        // End "tail" element
    } t_dll;

    #define DLL_HEAD(dll) dll->head
    #define DLL_TAIL(dll) dll->tail
    #define DLL_NEXT(e)   e->next
    #define DLL_PREV(e)   e->prev

    t_dll *dll_init(void);
    void dll_free(t_dll *dll);
    t_dll_element *dll_prepend(t_dll *dll, void *data);
    t_dll_element *dll_append(t_dll *dll, void *data);
    t_dll_element *dll_insert_before(t_dll *dll, t_dll_element *, void *data);
    t_dll_element *dll_insert_after(t_dll *dll, t_dll_element *, void *data);
    int dll_remove(t_dll *dll, t_dll_element *element);
    void dll_push(t_dll *dll, void *data);
    void *dll_pop(t_dll *dll);
    void *dll_top(t_dll *dll);

#endif

