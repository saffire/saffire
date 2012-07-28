%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
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
    yylval.sVal = strdup(yytext+1);
    if (yylval.sVal[yyleng-2] != yytext[0]) {
        fprintf(stderr,"improperly terminated string");
    } else {
        yylval.sVal[yyleng-2] = 0;
    }
    return T_STRING; }

\${label} {
    yylval.sVal = strdup(yytext);
    return T_VARIABLE; }

{label} {
    yylval.sVal = strdup(yytext);
    return T_LABEL; }

{lnum} {
    yylval.lVal = strtol(yytext, NULL, 10);
    return T_LNUM; }

{whitespace} { }


. {
    printf ("Unknown token: '%s'\n", yytext);
    yyterminate();
}

%%

int yywrap(void) { return 1; }
