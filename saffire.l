%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YYSTYPE char *
YYSTYPE yylval;

#include "parser.tab.h"

%}


lnum        [0-9]+
dnum        ([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)
exp_dnum    (({lnum}|{dnum})[eE][+-]?{lnum})
hnum        "0x"[0-9a-fA-F]+
bnum        "0b"[01]+
label       [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
whitespace  [ \t\n\r]+
tabspace    [ \t]*
tokens      [;:,.\[\]()|^&+-/*=%!~$<>?@]
any_char    [^]
newline     ("\n")
sl_comment  "//".+
ml_comment  "/*"([^\*]|\*[^/])*"*/"

opening_tags    [\"\']
closing_tags    [\"\']


%%

{ml_comment}|{sl_comment} { }

[\(\)\{\}:;=,]    { return yytext[0]; }

"program" {
    return T_PROGRAM; }

"++" {
    return T_INC; }

"--" {
    return T_DEC; }

"print" {
    return T_PRINT; }

{opening_tags}[^\"]*{closing_tags} {
    yylval=strdup(yytext);
    return T_STRING; }

\${label} {
    yylval=strdup(yytext);
    return T_VARIABLE; }

{label} {
    yylval=strdup(yytext);
    return T_LABEL; }

{lnum} {
    yylval = strdup(yytext);
    return T_LNUM; }

{whitespace} { }


. {
    printf ("Unknown token: '%s'\n", yytext);
    yyterminate();
}

%%

int yywrap(void) { return 1; }
