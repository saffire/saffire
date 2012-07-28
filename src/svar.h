#ifndef __SVAR_H__
#define __SVAR_H__

    /*
     * svars are omparable with php's zval
     *
     */


    /* Svars are stored in a fixed lookup table. We can only have
     * this many variables in our program */
    #define MAX_VARS 50


    // Constant defines for different sval types
    extern const int SV_NULL;
    extern const int SV_LONG;
    extern const int SV_STRING;
    extern const int SV_DOUBLE;

    typedef struct _svar {
        char type;          // Type of the variable
        char *name;         // Name of the variable
        union {
            long   l;       // Numerical variable
            double d;       // Double value
            char*  s;       // String value
        } val;
    } svar;



    void svar_init_table();
    svar *svar_alloc(char type, char *name, void *val);
    svar *svar_find(char *name);
    void svar_print(svar *var);

#endif