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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "general/output.h"
#include "general/config.h"
#include "fastcgi/scoreboard.h"
#include "fastcgi/daemonize.h"
#include "fastcgi/fastcgi_srv.h"

/**
 * Heavily based on the spawn-fcgi, http://cgit.stbuehler.de/gitosis/spawn-fcgi/
 */

#include "fcgiapp.h"

// This file descriptor has to be connected to a socket, otherwise
// FastCGI does not recognize this as a FastCGI program
#define FCGI_LISTENSOCK_FILENO 0

extern int sock_fd;


// Boolean flag
static int terminated = 0;

// Holds the number of workers we need to spawn. Protected by the scoreboard mutex (@TODO: change this into own mutex)
static int needs_spawn = 0;

FCGX_Stream *fcgi_in, *fcgi_out, *fcgi_err;
FCGX_ParamArray fcgi_env;

/**
 * Getenv for fcgi vars
 */
char *fcgi_getenv(const char *key) {
    char **p, *c;

    for (p = fcgi_env; (c = *p) != NULL; ++p) {
        if (strncmp(c, key, strlen(key)) == 0 && c[strlen(key)] == '=') {
            return c + strlen(key) + 1;
        }
    }
    return NULL;
}


/**
 * Make sure this loop does not return
 */
static int fcgi_loop(void) {
    // Move socket to the socket needed for FastCGI
    if (sock_fd != FCGI_LISTENSOCK_FILENO) {
        close(FCGI_LISTENSOCK_FILENO);
        dup2(sock_fd, FCGI_LISTENSOCK_FILENO);
        close(sock_fd);
        sock_fd = FCGI_LISTENSOCK_FILENO;
    }
    atexit(&FCGX_Finish);

    int ret;
    while (ret = FCGX_Accept(&fcgi_in, &fcgi_out, &fcgi_err, &fcgi_env), ret >= 0) {

        // Update scoreboard info
        scoreboard_lock();
        scoreboard->reqs++;
        pid_t cp = getpid();
        for (int i=0; i!=scoreboard->num_workers; i++) {
            if (scoreboard->workers[i].pid == cp) {
                scoreboard->workers[i].reqs++;
            }
        }
        scoreboard_unlock();

        char *qs = fcgi_getenv("QUERY_STRING");
        if (strcmp(qs, "raw") == 0) {
            FCGX_FPrintF(fcgi_out, "Content-type: text/html\r\n\r\n");

            FCGX_FPrintF(fcgi_out, "Worker PID: %d\n", getpid());
            char **env;
            for (env = fcgi_env; *env; env++) {
                char *key = *env;
                char *val = strchr(key, '=');
                if (val == NULL) {
                    val = "(empty)";
                } else {
                    *val = '\0';
                    val++;
                }
                FCGX_FPrintF(fcgi_out, "%s : %s\n", key, val);
            }

        } else {
            FCGX_FPrintF(fcgi_out, "Content-type: text/html\r\n"
                   "\r\n"
                   "<html><head><title>Saffire FastCGI Server</title></head>\n"
                   "<body>\n"
                   "<h1><blink><marquee>Hello world</marquee></blink></h1>\n");

            FCGX_FPrintF(fcgi_out, "<table border=1 align=center><tr><th colspan=2>Environment</th></tr>");
            FCGX_FPrintF(fcgi_out, "<tr><td>Worker PID</td><td>%d</td></tr>", getpid());
            char **env;
            for (env = fcgi_env; *env; env++) {
                char *key = *env;
                char *val = strchr(key, '=');
                if (val == NULL) {
                    val = "(empty)";
                } else {
                    *val = '\0';
                    val++;
                }
                FCGX_FPrintF(fcgi_out, "<tr><td>%s</td><td>%s</td></tr>", key, val);
            }
            FCGX_FPrintF(fcgi_out, "</table>");

            FCGX_FPrintF(fcgi_out, "<br><br><address>This page is powered by the Saffire FastCGI daemon.</address>");

            FCGX_FPrintF(fcgi_out, "</body>"
                    "</html>");
        }
    }

    exit(0);
}


/**
 * Called when a child terminates
 */
static void sighandler_child(int sig) {
    pid_t pid = wait(NULL);
    printf("CHILD signal %d found for pid: %d\n", sig, pid);

    // We need to spawn a new child, since this one has ended
    scoreboard_lock();
    needs_spawn++;
    scoreboard_unlock();


    // Update status of the current PID in the scoreboard
    scoreboard_lock();
    for (int i=0; i!=scoreboard->num_workers; i++) {
        if (scoreboard->workers[i].pid == pid) {
            scoreboard->workers[i].status = SB_ST_FREE;
        }
    }
    scoreboard_unlock();
}


/**
 * Called when the master terminates (will terminate all children as well)
 */
static void sighandler_term(int sig) {
    // Send TERM signal to all workers
    for (int i=0; i!=scoreboard->num_workers; i++) {
        if (scoreboard->workers[i].status == SB_ST_INUSE && scoreboard->workers[i].pid) {
            printf("Sending TERM signal to pid: %d\n", scoreboard->workers[i].pid);
            kill(SIGTERM, scoreboard->workers[i].pid);
        }
    }

    // We do not need to spawn any workers
    scoreboard_lock();
    needs_spawn = 0;
    scoreboard_unlock();

    // We can safely terminate
    terminated = 1;
}


/**
 * Simple SIGHUP handler
 */
static void sighandler_hup(int sig) {
    // Hangup signal detected
    scoreboard_dump();
}


/**
 * Spawn 'count' number of workers
 */
void spawn_worker(int count) {
    for (int i=0; i!=count; i++) {

        // Try and find a free slot in our scoreboard
        int slot = scoreboard_find_freeslot();
        if (slot == -1) continue; // Full scoreboard?

        // Fork process
        pid_t child_pid = fork_child();
        if (child_pid == 0) {
            // Reset signal handlers for worker
            signal(SIGHUP, SIG_DFL);
            signal(SIGTERM, SIG_DFL);

            // Do FastCGI loop
            fcgi_loop();

            // And we're done (no cleanup needed)
            exit(0);
        }

        // Parent process

        // Decrease our spawn counter
        scoreboard_lock();
        needs_spawn--;
        scoreboard_unlock();

        scoreboard_init_slot(slot, child_pid);
    }
}


/**
 * Daemonize master and spawn worker-processes
 */
void daemonize(void) {
    // Set new process group
    setsid();

//    // Close descriptors and open to /dev/null?
//    for (int i=getdtablesize(); i>=0; --i) close(i);
//    int i = open("/dev/null", O_RDWR); // stdin
//    dup(i); // stdout
//    dup(i); // stderr

    // Set correct umask for created files
    umask(027);

    // chroot?

    int worker_count = config_get_long("fastcgi.spawn_children", 10);
    if (worker_count <= 0) worker_count = 1;

    scoreboard_init(worker_count);
    needs_spawn = worker_count;

    // Set signals (after the workers have started
    signal(SIGCHLD, sighandler_child);      // Child handler
    signal(SIGTERM, sighandler_term);       // Termination handler
    signal(SIGHUP, sighandler_hup);         // Hangup handler

    // Repeat the master loop until terminated
    terminated = 0;
    while (! terminated) {
        // Spawn workers if needed
        if (needs_spawn > 0) spawn_worker(10);

        // Sleep until the next loop
        sleep(1);
    }

    // Daemon finished (SIGTERM). Do cleanup
    scoreboard_fini();
}



/**
 *
 */
int fastcgi_run(void) {
    // Make sure we don't run as a SUID root user
    if (check_suidroot() == -1) return 1;

    // Setup listening socket
    char *listen_dst = config_get_string("fastcgi.listen", "0.0.0.0:8000");
    if (setup_socket(listen_dst) == -1) return 1;

    // Drop privileges
    char *user = config_get_string("fastcgi.user", "-1");
    char *group = config_get_string("fastcgi.group", "-1");
    if (drop_privileges(user, group) == -1) return 1;

    // Check if we need to fork into the background
    int need_daemonizing = config_get_bool("fastcgi.daemonize", 1);

    if (need_daemonizing) {
        //@TODO: if ( !fork()) daemonize();
        daemonize();
    } else {
        fcgi_loop();
    }

    return 0;
}