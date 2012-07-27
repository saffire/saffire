#ifndef __SAFFIRE_PARSER_H__
#define __SAFFIRE_PARSER_H__

    void saffire_do_program_begin(char *title);
    void saffire_do_program_end();
    void saffire_do_assign(char *var_name, char *val);
    void saffire_do_print(char *str);
    void saffire_do_pre_inc(char *var_name);
    void saffire_do_post_inc(char *var_name);
    void saffire_do_pre_dec(char *var_name);
    void saffire_do_post_dec(char *var_name);
    void saffire_inner_statement();
    void saffire_do_expr();

#endif