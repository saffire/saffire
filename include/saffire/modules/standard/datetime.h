#ifndef __MODULE_STANDARD_DATETIME_H__
#define __MODULE_STANDARD_DATETIME_H__

    t_module module_datetime;

    typedef struct {
        time_t time;
    } t_datetime_data;

    typedef struct {
        SAFFIRE_OBJECT_HEADER
        t_datetime_data data;
        SAFFIRE_OBJECT_FOOTER
    } t_datetime_object;

#endif
