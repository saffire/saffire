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
#ifndef __CONFIG_H__
#define __CONFIG_H__

    /* Default INI settings, incuding comments */
    const char *default_ini[] = {
        "[global]",
        "  # debug, notice, warning, error",
        "  log.level = debug",
        "  log.path = /var/log/saffire/saffire.log"
        "",
        "",
        "[fastcgi]",
        "  pid.path = /var/run/saffire.pid",
        "  log.path = /var/log/saffire/fastcgi.log",
        "  # debug, notice, warning, error",
        "  log.level = notice",
        "  daemonize = true",
        "",
        "  listen = 0.0.0.0:80",
        "  listen.backlog = -1",
        "  listen.socket.user = nobody",
        "  listen.socket.group = nobody",
        "  listen.socket.mode = 0666",
        "",
        "  #status.url = /status",
        "  #ping.url = /ping",
        "  #ping.response = \"pong\"",
        ""
    };

    // Default INI file @TODO: platform specific!
    char global_ini_file[] = "/etc/saffire/saffire.ini";
    char user_ini_file[] = "~/saffire.ini";


    typedef struct {
        const char *log_level;
        const char *log_path;
    } t_config_global;

    typedef struct {
        const char *pid_path;
        const char *log_path;
        const char *log_level;
        int daemonize;
        const char *listen;
        const char *listen_backlog;
        const char *listen_socket_user;
        const char *listen_socket_group;
        const char *listen_socket_mode;
    } t_config_fastcgi;

    typedef struct {
        t_config_global global;
        t_config_fastcgi fastcgi;
    } t_config;


    // Global configuration settings
    t_config config;

#endif