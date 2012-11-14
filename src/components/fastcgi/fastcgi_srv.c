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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/socket.h>
#include <pwd.h>
#include <grp.h>
#include "general/config.h"

/**
 * Heavily based on the spawn-fcgi, http://cgit.stbuehler.de/gitosis/spawn-fcgi/
 */

// Environment will be overwritten by the the FASTCGI environment.
extern char **environ;


// Make sure that the FastCGI library does not redefine our printf()-like function
#define NO_FCGI_DEFINES
#include "fcgi_stdio.h"


// This file descriptor has to be connected to a socket, otherwise
// FastCGI does not recognize this as a FastCGI program
#define FCGI_LISTENSOCK_FILENO 0

int sock_fd = -1;       // Socket file descriptor
int pid_fd = -1;        // PID file descriptor
struct _sockdata {      // Socket data (either Unix or Inet)
    union {
        struct sockaddr_in in;
        struct sockaddr_un un;
    } data;
    int len;
} sockdata;

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

    int ret;
    while (ret = FCGI_Accept(), ret >= 0) {
        FCGI_printf("Content-type: text/html\r\n"
               "\r\n"
               "<html><head><title>Saffire FastCGI Server</title></head>\n"
               "<body>\n"
               "<h1><blink><marquee>Hello world</marquee></blink></h1>\n");

        FCGI_printf("<table border=1 align=center><tr><th colspan=2>Environment</th></tr>");
        char **env;
        for (env = environ; *env; env++) {
            char *key = *env;
            char *val = strchr(key, '=');
            if (val == NULL) {
                val = "(empty)";
            } else {
                *val = '\0';
                val++;
            }
            FCGI_printf("<tr><td>%s</td><td>%s</td></tr>", key, val);
        }
        FCGI_printf("</table>");

        FCGI_printf("<br><br><address>This page is powered by the Saffire FastCGI daemon.</address>");

        FCGI_printf("</body>"
               "</html>"
               );
    }
    exit(0);
}


/**
 * Drop privileges in case we are running as root
 */
static int drop_privileges(char *user, char *group) {
    // No root, so nothing to drop
    if (getuid() != 0) return 0;

    return 0;
}



static int setup_socket(char *socket_name) {
    // If the socketname starts with a /, it's a (absolute) path and thus a unix socket
    if (socket_name[0] == '/') {
        // Set socket info
        memset(&sockdata, 0, sizeof(struct sockaddr_un));
        sockdata.data.un.sun_family = AF_UNIX;
        strcpy(sockdata.data.un.sun_path, socket_name);
        sockdata.len = sizeof(sockdata.data.un.sun_family) + strlen(sockdata.data.un.sun_path);

        // unlink if needed
        unlink(socket_name);

        // Create socket
        if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            printf("Cannot create socket\n");
            return -1;
         }
    } else {
        sockdata.data.in.sin_family = AF_INET;
        //sockdata.data.in.sin_addr.s_addr
        //sockdata.data.in.sin_port
        sockdata.len = 0;
        //
        printf("Cannot use AF_INET yet");
        return -1;

    }



    // Set socket options
    int val = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
        close(sock_fd);
        printf("Cannot set SO_REUSEADDR on socket: %s\n", strerror(errno));
        return -1;
    }

    // Bind
    if (! bind(sock_fd, (struct sockaddr *) &sockdata.data, sockdata.len) < 0) {
        close(sock_fd);
        printf("Cannot bind() socket: %s\n", strerror(errno));
        return -1;
    }

    // Listen to the socket
    int backlog = config_get_long("fastcgi.listen.backlog", -1);
    if (backlog <= 0) backlog = 1024;
    if (listen (sock_fd, backlog) == -1) {
        close(sock_fd);
        printf("Cannot listen() on socket: %s\n", strerror(errno));
        return -1;
    }

    if (socket_name[0] == '/') {
        // Set socket permissions, user and group
        char *socket_user = config_get_string("fastcgi.listen.socket.user", -1);
        char *socket_group = config_get_string("fastcgi.listen.socket.group", -1);
        char *mode = config_get_string("fastcgi.listen.socket.mode", "0666");

        char *endptr;

        struct passwd *pwd;
        int uid = strtol(socket_user, &endptr, 10);
        if (*endptr != '\0') {
            pwd = getpwnam(socket_user);
            if (pwd == NULL) {
                printf("Cannot find user: %s: %s", socket_user, strerror(errno));
                return -1;
            }
            uid = pwd->pw_uid;
        }

        struct group *grp;
        int gid = strtol(socket_group, &endptr, 10);
        if (*endptr != '\0') {
            grp = getgrnam(socket_group);
            if (grp == NULL) {
                close(sock_fd);
                printf("Cannot find group: %s: %s", socket_group, strerror(errno));
                return -1;
            }
            gid = grp->gr_gid;
        }

        // Change owner of socket
        if (chown(socket_name, uid, gid) == -1) {
            close(sock_fd);
            printf("Cannot chown(): %s\n", strerror(errno));
            return -1;
        }

        // Change mode of socket
        mode_t modei = strtol(mode, NULL, 8);
        if (modei == 0) modei = 0600;  // RW for user if not correct settings
        if (chmod(socket_name, modei) == -1) {
            close(sock_fd);
            printf("Cannot chmod(): %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}


/**
 *
 */
static int open_pid(char *pid_path) {
    struct stat st;

    pid_fd = open(pid_path, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (pid_fd == 0) {
        return 0;
    }

    if (errno != EEXIST) {
        printf("Cannot open PID file: %s: %s\n", pid_path, strerror(errno));
        return -1;
    }

    if (stat(pid_path, &st) != 0) {
        printf("Cannot stat PID file: %s: %s\n", pid_path, strerror(errno));
        return -1;
    }

    if (!S_ISREG(st.st_mode)) {
        printf("PID is not a regular file: %s\n", pid_path);
    }

    pid_fd = open(pid_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (pid_fd == -1) {
        printf("Cannot open PID file: %s: %s\n", pid_path, strerror(errno));
        return -1;
    }

    return 0;
}


/**
 *
 */
static int daemonize(void) {
    pid_t child_pid = fork();

    if (child_pid == -1) {
        printf("Cannot fork(): %s", strerror(errno));
        return -1;
    }

    if (child_pid == 0) {
        setsid();
        return 0;
    }

    return 1;

    struct timeval tv = { 0, 5000 * 1000 };
    select(0, NULL, NULL, NULL, &tv);

    int ret;
    int status;
waitforchild:
    ret = waitpid(child_pid, &status, WNOHANG);
    if (ret == -1) {
        if (errno == EINTR) goto waitforchild;
        printf("Unknown error occured: %s\n", strerror(errno));
        return -1;
    }
    if (ret == 0) {
        printf("Child spawned: %d\n", child_pid);
        return child_pid;
    }
    if (WIFEXITED(status)) {
        return -1;
    }
}


/**
 *
 */
static int check_suidroot(void) {
    // Make sure we don't run as SUID root
    if (geteuid() == 0 || getegid() == 0) {
        printf("Please do not run this app with SUID root.\n");
        return -1;
    }

    return 1;
}


/**
 *
 */
int fastcgi_start(void) {
    // Make sure we don't run as a SUID root user
    if (check_suidroot() == -1) return 1;

    // Setup listening socket
    char *listen_dst = config_get_string("fastcgi.listen", "0.0.0.0:8000");
    if (setup_socket(listen_dst) == -1) return 1;

    // Setup PID
    char *pid_path = config_get_string("fastcgi.pid.path", "/tmp/saffire.pid");
    if (open_pid(pid_path) == -1) return 1;

    // Drop privileges
    char *user = config_get_string("fastcgi.user", "-1");
    char *group = config_get_string("fastcgi.group", "-1");
    if (drop_privileges(user, group) == -1) return 1;

    // Check if we need to daemonize
    int need_daemonizing = config_get_bool("fastcgi.daemonize", 1);

    if (! need_daemonizing) {
        // Don't daemonize
        fcgi_loop();
    }

    // Spawn process backgrounds
    int sc = config_get_long("fastcgi.spawn_children", 10);
    if (sc <= 0) sc = 1;

    for (int i=0; i!=sc; i++) {
        pid_t child_pid = daemonize();
        if (child_pid == -1) return 1;
        if (child_pid == 0) fcgi_loop();

        // Write PID to PID file if needed
        if (pid_fd > 0) {
            char strpid[10];
            sprintf(strpid, "%d\n", child_pid);
            if (i == sc- - 1) {
                i = write(pid_fd, strpid, strlen(strpid)-1);
            } else {
                i = write(pid_fd, strpid, strlen(strpid));
            }
        }
    }
    return 0;
}


int fastcgi_stop(void) {
    return 0;
}

int fastcgi_running(void) {
    return 0;
}

