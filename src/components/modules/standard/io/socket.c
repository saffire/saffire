/*
 Copyright (c) 2012-2015, The Saffire Group
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
#include <string.h>
#include <saffire/general/output.h>
#include <saffire/modules/module_api.h>
#include <saffire/modules/modules.h>
#include <saffire/objects/object.h>
#include <saffire/objects/objects.h>
#include <saffire/general/dll.h>
#include <saffire/general/smm.h>
#include <saffire/vm/vm.h>
#include <saffire/debug.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_socket, ctor) {
    t_numerical_object *family_obj;
    t_numerical_object *sockettype_obj;
    t_numerical_object *protocol_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "nnn", &family_obj, &sockettype_obj, &protocol_obj)) {
        return NULL;
    }

    // Setup values for the new object
    t_socket_object *socket_obj = (t_socket_object *)self;

    socket_obj->data.socket = socket(OBJ2NUM(family_obj), OBJ2NUM(sockettype_obj), OBJ2NUM(protocol_obj));
    socket_obj->data.socket_type = OBJ2NUM(family_obj);

    if (socket_obj->data.socket == -1) {
        object_raise_exception(Object_IoException, 1, "Cannot create socket: %s", strerror(errno));
        return NULL;
    }

    socket_obj->data.bytes_in = 0;
    socket_obj->data.bytes_out = 0;
    socket_obj->data.accepted = 0;
    socket_obj->data.server_addr = NULL;
    socket_obj->data.client_addr = NULL;

    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_socket, setOption) {
    t_numerical_object *level_obj;
    t_numerical_object *optname_obj;
    t_object *optval_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "nno", &level_obj, &optname_obj, &optval_obj)) {
        return NULL;
    }

    t_socket_object *socket_obj = (t_socket_object *)self;

    // @TODO: Not all options use numerical values!
    if (! OBJECT_IS_NUMERICAL(optval_obj)) {
        object_raise_exception(Object_ArgumentException, 1, "Value for setOption() must be numerical value");
        return NULL;
    }

    long optval = OBJ2NUM((t_numerical_object *)optval_obj);
    long optval_len = sizeof(optval);

    int res = setsockopt(socket_obj->data.socket, OBJ2NUM(level_obj), OBJ2NUM(optname_obj), (void *)&optval, optval_len);
    if (res == -1) {
        object_raise_exception(Object_IoException, 1, "Could not set socket option %s", strerror(errno));
        return NULL;
    }

    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_socket, getOption) {
    t_numerical_object *level_obj;
    t_numerical_object *optname_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "nn", &level_obj, &optname_obj)) {
        return NULL;
    }

    t_socket_object *socket_obj = (t_socket_object *)self;

    long optval;
    socklen_t optval_len = sizeof(optval);

    int res = getsockopt(socket_obj->data.socket, OBJ2NUM(level_obj), OBJ2NUM(optname_obj), (void *)&optval, &optval_len);
    if (res == -1) {
        object_raise_exception(Object_IoException, 1, "Could not get socket option: %s", strerror(errno));
        return NULL;
    }


    RETURN_NUMERICAL(optval);
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(io_socket, bind) {
    t_string_object *host_obj;
    t_numerical_object *port_obj;
    struct sockaddr_in sa;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "sn", &host_obj, &port_obj)) {
        return NULL;
    }

    t_socket_object *socket_obj = (t_socket_object *)self;

    if (socket_obj->data.socket_type != AF_INET) {
        object_raise_exception(Object_IoException, 1, "Current only able to bind AF_INET sockets");
        return NULL;
    }

    // Set values
    sa.sin_family = AF_INET;
    sa.sin_port = htons(OBJ2NUM(port_obj));
    // @TODO: We only support IP addresses. Make sure we could convert hostnames through gethostbyname()
    inet_pton(AF_INET, STROBJ2CHAR0(host_obj), &(sa.sin_addr));

    // Bind socket
    int res = bind(socket_obj->data.socket, (struct sockaddr *)&sa, sizeof(sa));
    if (res == -1) {
        object_raise_exception(Object_IoException, 1, "Cannot bind socket: %s", strerror(errno));
        return NULL;
    }

    // Allocate memory for struct sockaddr_in
    socket_obj->data.server_addr = smm_malloc(sizeof(sa));
    socket_obj->data.server_addr_len = sizeof(sa);
    memcpy(socket_obj->data.server_addr, &sa, sizeof(sa));


    RETURN_SELF;
}

/**
 *
 */
SAFFIRE_MODULE_METHOD(io_socket, listen) {
    t_numerical_object *backlog_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &backlog_obj)) {
        return NULL;
    }

    t_socket_object *socket_obj = (t_socket_object *)self;

    listen(socket_obj->data.socket, OBJ2NUM(backlog_obj));

    RETURN_SELF;
}


/**
 *
 */
SAFFIRE_MODULE_METHOD(io_socket, accept) {
    t_socket_object *socket_obj = (t_socket_object *)self;
    socklen_t sin_len;
    struct sockaddr_in sin;

    int res = accept(socket_obj->data.socket, (struct sockaddr *)&sin, &sin_len);

    // Non-blocking
    if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        RETURN_FALSE;
    }

    if (res == -1) {
        object_raise_exception(Object_IoException, 1, "Error while accept()ing the socket: %s", strerror(errno));
        return NULL;
    }

    // Res is the new socket, return a new instance

    // increase the number of accepted connections for this socket
    socket_obj->data.accepted++;

    t_socket_object *client_obj = (t_socket_object *)object_alloc_instance(Object_Socket, 0);
    client_obj->data.socket = res;
    client_obj->data.socket_type = socket_obj->data.socket_type;

    client_obj->data.bytes_in = 0;
    client_obj->data.bytes_out = 0;
    client_obj->data.accepted = 0;

    // Copy our connecting side as server addr
    client_obj->data.server_addr = smm_malloc(sin_len);
    memcpy(client_obj->data.server_addr, &sin, sin_len);
    client_obj->data.server_addr_len = sin_len;

    client_obj->data.client_addr = NULL;
    client_obj->data.client_addr_len = 0;

    RETURN_OBJECT(client_obj);
}

SAFFIRE_MODULE_METHOD(io_socket, close) {
    t_socket_object *socket_obj = (t_socket_object *)self;

    close(socket_obj->data.socket);
    socket_obj->data.socket = -1;
    socket_obj->data.socket_type = 0;

    RETURN_SELF;
}

SAFFIRE_MODULE_METHOD(io_socket, read) {
    t_numerical_object *bytes_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "n", &bytes_obj)) {
        return NULL;
    }

    int buflen = OBJ2NUM(bytes_obj);
    char *buf = smm_malloc(buflen);

    t_socket_object *socket_obj = (t_socket_object *)self;

    int read_bytes = recv(socket_obj->data.socket, buf, buflen, 0);
    t_string_object *obj = (t_string_object *)object_alloc_instance(Object_String, 2, read_bytes, buf);
    smm_free(buf);

    RETURN_OBJECT(obj);
}

SAFFIRE_MODULE_METHOD(io_socket, write) {
    t_string_object *str_obj;

    if (! object_parse_arguments(SAFFIRE_METHOD_ARGS, "s", &str_obj)) {
        return NULL;
    }

    t_socket_object *socket_obj = (t_socket_object *)self;

    int written_bytes = send(socket_obj->data.socket, STROBJ2CHAR0(str_obj), STROBJ2CHAR0LEN(str_obj), 0);

    RETURN_NUMERICAL(written_bytes);
}

SAFFIRE_MODULE_METHOD(io_socket, getHost) {
    RETURN_STRING_FROM_CHAR("nothing yet");
}

SAFFIRE_MODULE_METHOD(io_socket, getPort) {
    RETURN_NUMERICAL(123);
}





static void obj_free(t_object *obj) {
    t_socket_object *socket_obj = (t_socket_object *)obj;
    if (! socket_obj) return;

    if (socket_obj->data.socket != -1) {
        shutdown(socket_obj->data.socket, SHUT_RDWR);
        socket_obj->data.socket = -1;
    }
}

static void obj_destroy(t_object *obj) {
    smm_free(obj);
}

#ifdef __DEBUG
static char *obj_debug(t_object *obj) {
    t_socket_object *socket_obj = (t_socket_object *)obj;

    // If we CAN print debug info, we HAVE space for debug info. At the end of an object.

    // Store debug info at the end of an object. There is space allocated for this purpose in debug mode
    snprintf(socket_obj->__debug_info, DEBUG_INFO_SIZE-1, "socket[%d]", socket_obj->data.socket);
    return socket_obj->__debug_info;
}
#endif


// List object management functions
t_object_funcs socket_funcs = {
        NULL,                 // Populate a socket object
        obj_free,             // Free a socket object
        obj_destroy,          // Destroy a socket object
        NULL,                 // Clone
        NULL,                 // Cache
        NULL,                 // Hash
#ifdef __DEBUG
        obj_debug
#endif
};



// Initial object
t_socket_object io_socket_struct = {
    OBJECT_HEAD_INIT("socket", objectTypeUser, OBJECT_TYPE_CLASS, &socket_funcs, sizeof(t_socket_object_data)),
    {
        -1,      /* socket handle */
        0,      /* bytes in */
        0       /* bytes out */
    },
    OBJECT_FOOTER
};

static void _init(void) {
    io_socket_struct.attributes = ht_create();

    object_add_internal_method((t_object *)&io_socket_struct, "__ctor",     ATTRIB_METHOD_CTOR, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_ctor);
    object_add_internal_method((t_object *)&io_socket_struct, "setOption",  ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_setOption);
    object_add_internal_method((t_object *)&io_socket_struct, "getOption",  ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_getOption);

    object_add_internal_method((t_object *)&io_socket_struct, "bind",       ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_bind);
    object_add_internal_method((t_object *)&io_socket_struct, "listen",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_listen);
    object_add_internal_method((t_object *)&io_socket_struct, "accept",     ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_accept);

    object_add_internal_method((t_object *)&io_socket_struct, "close",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_close);
    object_add_internal_method((t_object *)&io_socket_struct, "read",    ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_read);
    object_add_internal_method((t_object *)&io_socket_struct, "write",   ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_write);

    object_add_internal_method((t_object *)&io_socket_struct, "getHost", ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_getHost);
    object_add_internal_method((t_object *)&io_socket_struct, "getPort", ATTRIB_METHOD_NONE, ATTRIB_VISIBILITY_PUBLIC, module_io_socket_method_getPort);

    // Socket constants
    object_add_constant((t_object *)&io_socket_struct, "AF_UNIX",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(AF_UNIX));
    object_add_constant((t_object *)&io_socket_struct, "AF_INET",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(AF_INET));
    object_add_constant((t_object *)&io_socket_struct, "AF_INET6",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(AF_INET6));

    object_add_constant((t_object *)&io_socket_struct, "SOCK_STREAM",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SOCK_STREAM));
    object_add_constant((t_object *)&io_socket_struct, "SOCK_DGRAM",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SOCK_DGRAM));
    object_add_constant((t_object *)&io_socket_struct, "SOCK_RAW",     ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SOCK_RAW));

    object_add_constant((t_object *)&io_socket_struct, "SO_DEBUG",      ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_DEBUG));
    object_add_constant((t_object *)&io_socket_struct, "SO_REUSEADDR",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_REUSEADDR));
#ifdef SO_REUSEPORT
    object_add_constant((t_object *)&io_socket_struct, "SO_REUSEPORT",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_REUSEPORT));
#endif
    object_add_constant((t_object *)&io_socket_struct, "SO_KEEPALIVE",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_KEEPALIVE));
    object_add_constant((t_object *)&io_socket_struct, "SO_DONTROUTE",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_DONTROUTE));
    object_add_constant((t_object *)&io_socket_struct, "SO_LINGER",     ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_LINGER));
    object_add_constant((t_object *)&io_socket_struct, "SO_BROADCAST",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_BROADCAST));
    object_add_constant((t_object *)&io_socket_struct, "SO_OOBINLINE",  ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_OOBINLINE));
    object_add_constant((t_object *)&io_socket_struct, "SO_SNDBUF",     ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_SNDBUF));
    object_add_constant((t_object *)&io_socket_struct, "SO_RCVBUF",     ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_RCVBUF));
    object_add_constant((t_object *)&io_socket_struct, "SO_SNDLOWAT",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_SNDLOWAT));
    object_add_constant((t_object *)&io_socket_struct, "SO_RCVLOWAT",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_RCVLOWAT));
    object_add_constant((t_object *)&io_socket_struct, "SO_SNDTIMEO",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_SNDTIMEO));
    object_add_constant((t_object *)&io_socket_struct, "SO_RCVTIMEO",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_RCVTIMEO));
    object_add_constant((t_object *)&io_socket_struct, "SO_TYPE",       ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_TYPE));
#ifdef SO_FAMILY
    object_add_constant((t_object *)&io_socket_struct, "SO_FAMILY",     ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_FAMILY));
#endif
    object_add_constant((t_object *)&io_socket_struct, "SO_ERROR",      ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_ERROR));
#ifdef SO_BINDTODEVICE
    object_add_constant((t_object *)&io_socket_struct, "SO_BINDTODEVICE", ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SO_BINDTODEVICE));
#endif
    object_add_constant((t_object *)&io_socket_struct, "SOL_SOCKET",    ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SOL_SOCKET));
    object_add_constant((t_object *)&io_socket_struct, "SOMAXCONN",     ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(SOMAXCONN));
#ifdef TCP_NODELAY
    object_add_constant((t_object *)&io_socket_struct, "TCP_NODELAY",   ATTRIB_VISIBILITY_PUBLIC, NUM2OBJ(TCP_NODELAY));
#endif

}

static void _fini(void) {
    object_free_internal_object((t_object *)&io_socket_struct);
}

static t_object *_objects[] = {
    (t_object *)&io_socket_struct,
    NULL
};

t_module module_io_socket = {
    "\\saffire\\io",
    "Standard socket I/O module",
    _objects,
    _init,
    _fini,
};
