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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "fastcgi/scoreboard.h"

int shm_id;     // ID of shared memory
int sem_id;     // ID of semaphore (for locking shared memory)

t_scoreboard *scoreboard;       // Pointer to scoreboard struct in SHM


/**
 * Initialize scoreboard with a N amount of workers
 */
int scoreboard_init(int workers) {
    // Create shared segment
    shm_id = shmget(IPC_PRIVATE, sizeof(t_scoreboard) + ( sizeof(t_worker_scoreboard) * workers), IPC_CREAT | 0666);
    if (shm_id < 0) {
        error("Cannot get shared memory segment: %s\n", strerror(errno));
        return -1;
    }
    scoreboard = shmat(shm_id, (void *)0, 0);
    if (scoreboard == -1) {
        error("Cannot connect to shared memory: %s\n", strerror(errno));
        return -1;
    }

    // Create semaphore
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id < 0) {
        error("Cannot create semaphore: %s\n", strerror(errno));
        return -1;
    }

    // Init scoreboard structure
    scoreboard = shm_id;
    scoreboard->start_ts = time();
    scoreboard->reqs = 0;
    scoreboard->num_workers = workers;
    for (int i=0; i!=workers; i++) {
        scoreboard->workers[i].pid = 0;
        scoreboard->workers[i].status = 0;
        scoreboard->workers[i].reqs = 0;
        scoreboard->workers[i].bytes_in = 0;
        scoreboard->workers[i].bytes_out = 0;
    }
}


/**
 * Lock scoreboard. Blocking!
 */
int scoreboard_lock(void) {
    struct sembuf sb;

    sb.sem_num = 0;
    sb.sem_op = -1;

    semop(sem_id, &sb, 1);
}

/**
 * Unlock scoreboard.
 */
int scoreboard_unlock(void) {
    struct sembuf sb;

    sb.sem_num = 0;
    sb.sem_op = 1;

    semop(sem_id, &sb, 1);
}


/**
 * remove scoreboard
 */
int scoreboard_fini(void) {
    // Remove shared segment
    shmctl(shm_id, IPC_RMID, NULL);
}

/**
 * Fetch a worker by PID
 */
t_worker_scoreboard *scoreboard_worker_fetch(pid_t *pid) {
    for (int i=0; i!=scoreboard->num_workers; i++) {
        if (scoreboard->workers[i].pid == pid) {
            return scoreboard->workers[i];
        }
    }
    return NULL;
}