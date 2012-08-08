%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "parser.tab.h"

%}

%option yylineno

lnum            [0-9]+
identifier      [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
whitespace      [ \t\n\r]+
sl_comment      "//".+
ml_comment      "/*"([^\*]|\*[^/])*"*/"
opening_tags    [\"\']
closing_tags    [\"\']

%%

{ml_comment}|{sl_comment} { }
{whitespace} { }

[-+<>\(\)\{\}:;=,\.\[\]\?]    { return yytext[0]; }


">="        return T_GE;
"<="        return T_LE;
"=="        return T_EQ;
"!="        return T_NE;

"||"        return T_OR;
"&&"        return T_AND;

"<<"        return T_SHIFT_LEFT;
">>"        return T_SHIFT_RIGHT;

"+="        return T_PLUS_ASSIGNMENT;
"-="        return T_MINUS_ASSIGNMENT;
"*="        return T_MUL_ASSIGNMENT;
"/="        return T_DIV_ASSIGNMENT;
"%="        return T_MOD_ASSIGNMENT;
"&="        return T_AND_ASSIGNMENT;
"|="        return T_OR_ASSIGNMENT;
"^="        return T_XOR_ASSIGNMENT;
"<<="       return T_SL_ASSIGNMENT;
">>="       return T_SR_ASSIGNMENT;

"while"     return T_WHILE;
"if"        return T_IF;
"else"      return T_ELSE;
"use"       return T_USE;
"as"        return T_AS;
"do"        return T_DO;
"for"       return T_FOR;
"switch"    return T_SWITCH;

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

"catch"         return T_CATCH;
"finally"       return T_FINALLY;
"throw"         return T_THROW;
"return"        return T_RETURN;
"break"         return T_BREAK;
"breakelse"     return T_BREAKELSE;
"continue"      return T_CONTINUE;
"try"           return T_TRY;
"default"       return T_DEFAULT;
"goto"          return T_GOTO;

"self"          return T_SELF;
"parent"        return T_PARENT;

{opening_tags}[^\"]*{closing_tags} {
    yylval.sVal = strdup(yytext+1);
    if (yylval.sVal[yyleng-2] != yytext[0]) {
        fprintf(stderr,"improperly terminated string");
    } else {
        yylval.sVal[yyleng-2] = 0;
    }
    return T_STRING; }

\${identifier} {
    yylval.sVal = strdup(yytext);
    return T_VARIABLE; }

{identifier} {
    yylval.sVal = strdup(yytext);
    return T_IDENTIFIER;
}

{identifier}[!?]? {
    yylval.sVal = strdup(yytext);
    return T_IDENTIFIER_METHOD;
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
