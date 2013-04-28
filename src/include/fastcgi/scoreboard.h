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
#ifndef __SCOREBOARD_H__
#define __SCOREBOARD_H__

    #include <sys/types.h>

    #define SB_ST_FREE     0        // Slot is free
    #define SB_ST_INIT     1        // Slot is currently being filled
    #define SB_ST_INUSE    2        // Slot is in use

    typedef struct _worker_scoreboard {
        char    status;         // Not yet used
        pid_t   pid;            // PID of the worker
        long    reqs;           // Number of requests served
        long    bytes_in;       // Number of bytes in
        long    bytes_out;      // Number of bytes out
    } t_worker_scoreboard;

    typedef struct _scoreboard {
        int     start_ts;               // When has it started?
        long    reqs;                   // How many requests did it handle?
        int     num_workers;            // How many workers are present?
        t_worker_scoreboard workers[];  // Our actual workers
    } t_scoreboard;

    t_scoreboard *scoreboard;       // Pointer to scoreboard struct in SHM


    int scoreboard_init(int workers);
    int scoreboard_fini(void);
    void scoreboard_init_slot(int slot, pid_t pid);
    t_scoreboard *scoreboard_fetch(void);
    void scoreboard_dump(void);
    int scoreboard_find_freeslot(void);

    int scoreboard_lock(void);
    int scoreboard_unlock(void);

#endif
