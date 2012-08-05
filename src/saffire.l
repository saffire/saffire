%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "parser.tab.h"

%}

%option yylineno

lnum            [0-9]+
label           [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
whitespace      [ \t\n\r]+
sl_comment      "//".+
ml_comment      "/*"([^\*]|\*[^/])*"*/"
opening_tags    [\"\']
closing_tags    [\"\']

%%

{ml_comment}|{sl_comment} { }
{whitespace} { }

[-+<>\(\)\{\}:;=,\.\[\]]    { return yytext[0]; }


">="        return T_GE;
"<="        return T_LE;
"=="        return T_EQ;
"!="        return T_NE;

"+="        return T_PLUS_EQUAL;
"-="        return T_MINUS_EQUAL;
"*="        return T_MUL_EQUAL;
"/="        return T_DIV_EQUAL;
"%="        return T_MOD_EQUAL;
"&="        return T_AND_EQUAL;
"|="        return T_OR_EQUAL;
"^="        return T_XOR_EQUAL;
"<<="       return T_SL_EQUAL;
">>="       return T_SR_EQUAL;

"while"     return T_WHILE;
"if"        return T_IF;
"else"      return T_ELSE;
"use"       return T_USE;
"as"        return T_AS;

"class"         return T_CLASS;
"extends"       return T_EXTENDS;
"implements"    return T_IMPLEMENTS;
"abstract"      return T_ABSTRACT;
"final"         return T_FINAL;
"interface"     return T_INTERFACE;
"const"         return T_CONST;
"static"        return T_STATIC;

"public"        return T_PUBLIC;
"private"       return T_PRIVATE;
"protected"     return T_PROTECTED;
"method"        return T_METHOD;

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
    return T_LABEL;
}

{label}[!?]? {
    yylval.sVal = strdup(yytext);
    return T_LABEL_METHOD;
}

{lnum} {
    yylval.lVal = strtol(yytext, NULL, 10);
    return T_LNUM; }

. {
    printf ("Unknown token: '%s'\n", yytext);
    yyterminate();
}

%%

int yywrap(void) { return 1; }
