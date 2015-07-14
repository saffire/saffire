#ifndef __MODULE_STANDARD_OS_H__
#define __MODULE_STANDARD_OS_H__

    t_module module_os;

    typedef struct {
        struct stat sb;
    } t_stat_object_data;

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        t_stat_object_data data;
        SAFFIRE_OBJECT_FOOTER
    } t_stat_object;

    t_stat_object Object_Stat_struct;

    #define Object_Stat   (t_object *)&Object_Stat_struct

#endif
