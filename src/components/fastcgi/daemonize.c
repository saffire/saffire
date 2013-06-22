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
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
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
#include "general/output.h"
#include "general/config.h"
#include "fastcgi/scoreboard.h"
#include "fastcgi/daemonize.h"
#include "debug.h"


int sock_fd = -1;       // Socket file descriptor

/**
 * Drop privileges in case we are running as root
 */
int drop_privileges(char *user, char *group) {
    // No root, so nothing to drop
    if (getuid() != 0) return 0;

    // Find user by name or id
    struct passwd *pwd;
    pwd = getpwnam(user);
    if (pwd == NULL) {
        pwd = getpwuid(atoi(user));
        if (pwd == NULL || atoi(user) == 0) {
            fatal_error(1, "Cannot find user '%s'\n", user);
            return -1;
        }
    }

    // Find group by name or id
    struct group *grp;
    grp = getgrnam(group);
    if (grp == NULL) {
        grp = getgrgid(atoi(group));
        if (grp == NULL || atoi(group) == 0) {
            fatal_error(1, "Cannot find group '%s'\n", group);
            return -1;
        }
    }

    // Drop to group privs
    if (setgid(grp->gr_gid) != 0) {
        fatal_error(1, "setgid() failed: %s\n", strerror(errno));
        return -1;
    }

    // Drop to user privs
    if (setuid(pwd->pw_uid) != 0) {
        fatal_error(1, "setuid() failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}



int setup_socket(char *socket_name) {
    // If the socketname starts with a /, it's a (absolute) path and thus a unix socket
    if (socket_name[0] == '/') {
        // Set socket info
        memset(&sockdata, 0, sizeof(struct sockaddr_un));
        sockdata.data.un.sun_family = AF_UNIX;
        strcpy(sockdata.data.un.sun_path, socket_name);
        sockdata.len = sizeof(sockdata.data.un.sun_family) + strlen(sockdata.data.un.sun_path);

        // unlink if needed
        unlink(socket_name);
    } else {
        memset(&sockdata, 0, sizeof(struct sockaddr_in));
        sockdata.data.in.sin_family = AF_INET;

        char *colon = strchr(socket_name, ':');
        if (colon == NULL) {
            fatal_error(1, "bind address should be in <host|ip>:<port> format\n");
            return -1;
        }

        char *addr = strndup(socket_name, colon-socket_name);
        int port = strtol(colon+1, NULL, 10);
        if (port < 1 || port > 65535) {
            fatal_error(1, "Incorrect value for port specified\n");
            return -1;
        }

        sockdata.data.in.sin_port = htons(port);
        sockdata.data.in.sin_addr.s_addr = inet_addr(addr);

        if (sockdata.data.in.sin_addr.s_addr == -1) {
            fatal_error(1, "Incorrect value for addr specified\n");
            return -1;
        }
        sockdata.len = sizeof(sockdata.data.in);
    }


    // Create socket
    if ((sock_fd = socket(sockdata.data.in.sin_family, SOCK_STREAM, 0)) < 0) {
        fatal_error(1, "Cannot create socket\n");
        return -1;
    }


    // Set socket options
    int val = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0) {
        close(sock_fd);
        fatal_error(1, "Cannot set SO_REUSEADDR on socket: %s\n", strerror(errno));
        return -1;
    }

    // Bind
    if (bind(sock_fd, (struct sockaddr *) &sockdata.data, sockdata.len) != 0) {
        close(sock_fd);
        fatal_error(1, "Cannot bind() socket: %s\n", strerror(errno));
        return -1;
    }

    // Listen to the socket
    int backlog = config_get_long("fastcgi.listen.backlog", -1);
    if (backlog <= 0) backlog = 1024;
    if (listen (sock_fd, backlog) == -1) {
        close(sock_fd);
        fatal_error(1, "Cannot listen() on socket: %s\n", strerror(errno));
        return -1;
    }

    if (IS_UNIXSOCK(sockdata)) {
        // Set socket permissions, user and group
        char *socket_user = config_get_string("fastcgi.listen.socket.user", "-1");
        char *socket_group = config_get_string("fastcgi.listen.socket.group", "-1");
        char *mode = config_get_string("fastcgi.listen.socket.mode", "0666");

        char *endptr;

        if (getuid() == 0) {
            struct passwd *pwd;
            int uid = strtol(socket_user, &endptr, 10);
            if (*endptr != '\0') {
                pwd = getpwnam(socket_user);
                if (pwd == NULL) {
                    fatal_error(1 ,"Cannot find user: %s: %s", socket_user, strerror(errno));
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
                    fatal_error(1, "Cannot find group: %s: %s", socket_group, strerror(errno));
                    return -1;
                }
                gid = grp->gr_gid;
            }

            // Change owner of socket
            if (chown(socket_name, uid, gid) == -1) {
                close(sock_fd);
                fatal_error(1, "Cannot chown(): %s\n", strerror(errno));
                return -1;
            }

        }

        // Change mode of socket
        mode_t modei = strtol(mode, NULL, 8);
        if (modei == 0) modei = 0600;  // RW for user if not correct settings
        if (chmod(socket_name, modei) == -1) {
            close(sock_fd);
            fatal_error(1, "Cannot chmod(): %s\n", strerror(errno));
            return -1;
        }
    }

    return 0;
}


/**
 *
 */
int fork_child(void) {
    pid_t child_pid = fork();

    if (child_pid == -1) {
        fatal_error(1, "Cannot fork(): %s", strerror(errno));
        return -1;
    }

    return child_pid;
}


/**
 *
 */
int check_suidroot(void) {
    // Make sure we don't run as SUID root, but running as root is allowed (since we can drop our privs then)
    if (getuid() != 0 && (geteuid() == 0 || getegid() == 0)) {
        fatal_error(1, "Please do not run this app with SUID root.\n");
        return -1;
    }

    return 1;
}
