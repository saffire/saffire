#ifndef __MODULE_STANDARD_IO_SOCKET_H__
#define __MODULE_STANDARD_IO_SOCKET_H__

    t_module module_io_socket;

    typedef struct {
        int     socket;         // Actual socket handle
        int     socket_type;    // Socket type (AF_*)
        long    bytes_in;       // Bytes read in
        long    bytes_out;      // Bytes read out
        long    accepted;       // Accepted connections

        // Socket info for the server side
        struct sockaddr *server_addr;
        socklen_t server_addr_len;

        // Socket info from the client side (if connected, NULL otherwise)
        struct sockaddr *client_addr;
        socklen_t *client_addr_len;
    } t_socket_object_data;

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        t_socket_object_data data;
        SAFFIRE_OBJECT_FOOTER
    } t_socket_object;

    t_socket_object io_socket_struct;

    #define Object_Socket   (t_object *)&io_socket_struct

#endif
