/*
 Copyright (c) 2012, Joshua Thijssen
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "parser.tab.h"

%}

%option yylineno

lnum            [0-9]+
identifier      (::[\+])|([\$a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*[\?\!]?)
whitespace      [ \t\n\r]+
sl_comment      "//".+
ml_comment      "/*"([^\*]|\*[^/])*"*/"

%%

{ml_comment}|{sl_comment} { }
{whitespace} { }

[-+\%\/<>\(\)\{\}:;=,\.\[\]\?!\*^]    { return yytext[0]; }

"=~"        return T_RE;
">="        return T_GE;
"<="        return T_LE;
"=="        return T_EQ;
"!="        return T_NE;
"in"        return T_IN;

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

"++"        return T_OP_INC;
"--"        return T_OP_DEC;

"while"     return T_WHILE;
"if"        return T_IF;
"else"      return T_ELSE;
"use"       return T_USE;
"as"        return T_AS;
"do"        return T_DO;
"for"       return T_FOR;
"foreach"   return T_FOREACH;
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
"readonly"      return T_READONLY;
"property"      return T_PROPERTY;

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
"case"          return T_CASE;

"self"          return T_SELF;
"parent"        return T_PARENT;


\"[^\"]*\" {
    yylval.sVal = strdup(yytext+1);
    if (yylval.sVal[yyleng-2] != yytext[0]) {
        fprintf(stderr,"improperly terminated string");
    } else {
        yylval.sVal[yyleng-2] = 0;
    }
    return T_STRING; }

\'[^\']*\' {
    yylval.sVal = strdup(yytext+1);
    if (yylval.sVal[yyleng-2] != yytext[0]) {
        fprintf(stderr,"improperly terminated string");
    } else {
        yylval.sVal[yyleng-2] = 0;
    }
    return T_STRING; }

{identifier} {
    yylval.sVal = strdup(yytext);
    return T_IDENTIFIER;
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
