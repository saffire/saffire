#ifndef __MODULE_STANDARD_IO_FILE_H__
#define __MODULE_STANDARD_IO_FILE_H__

    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>

    t_module module_io_file;


    typedef struct {
        FILE            *fp;            // Actual file pointer
        char            *path;          // File path to file
        long            bytes_in;       // bytes read in
        long            bytes_out;      // bytes read out
        t_hash_table    *stat;          // Hashtable with stat information
    } t_file_object_data;

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        t_file_object_data data;
        SAFFIRE_OBJECT_FOOTER
    } t_file_object;

    t_file_object Object_File_struct;

    #define Object_File   (t_object *)&Object_File_struct

#endif
