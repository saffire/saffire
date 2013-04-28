/*
 Copyright (c) 2012-2013, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
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
#include <unistd.h>

#include "general/output.h"
#include "general/smm.h"
#include "compiler/ast_nodes.h"
#include "compiler/parser.tab.h"

#define YY_NO_INPUT 1
#define YYPRINT 1

#define step                                                \
    do {                                                    \
        yylloc.first_column   = yylloc.last_column + 1;     \
        yylloc.last_column += strlen(yytext);               \
    } while(0)

#define step_nl                                             \
    do {                                                    \
        yylloc.first_line = yylloc.last_line = yylineno;    \
        yylloc.first_column = 1;                            \
        yylloc.last_column = 0;                             \
    } while (0)

static char *strbuf;

/* Create yy_*_state wrappers */
static void yy_push_state (int new_state);
static void yy_pop_state (void );

void saffire_push_state(int state) {
    yy_push_state(state);
}

char *add_strbuf(char *buf, char *s) {
    if (buf == NULL) {
        // Realloc does not clear our buf, so we need to copy it, not cat
        buf = smm_malloc(strlen(s)+1);
        strcpy(buf, s);
    } else {
        buf = smm_realloc(buf, strlen(buf) + strlen(s)+1);
        strcat(buf, s);
    }
    return buf;
}
char *add_strbuf_char(char *buf, char c) {
    char s[2];
    s[0] = c;
    s[1] = 0;
    return add_strbuf(buf, s);
}

void saffire_pop_state() {
    yy_pop_state();
}

%}

/* Define lexer states */
%s st_regex
%s st_div

/* Important: exclusive states! Matches no other lexems (like identifier, whitespace etc) */
%x st_string_dbl
%x st_string_sgl

%option stack
%option yylineno

/* We define our own yymalloc functionality */
%option noyyfree noyyrealloc noyyalloc

/* suppres -Wall warning about yyunput */
%option nounput noyy_top_state


/* Our actual lexemes */
lnum            [0-9]+
/* identifier      (::[\+])|([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)|([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*[\?\!]?) */
identifier      (::[\+])|([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)|([a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*)
whitespace      [ \t\r]+
newline         "\n"
sl_comment      "//".+
ml_comment      "/*"([^\*]|\*[^/])*"*/"

%%

{ml_comment}|{sl_comment}   { }
{whitespace}                { }
{newline}                   { step_nl; }

<st_div>\/ { return '/'; }

[-+\%\/<>\(\)\{\}:;,\.\[\]\?!\*^\|]    { step; saffire_push_state(st_regex); return yytext[0]; }


    /* Only match regex when we are in the regex state */
<st_regex>\/[^\/\\]*(?:\\.[^\/\\]*)*\/[a-z]*    {
    step;
    yylval.sVal = smm_strdup(yytext);
    return T_REGEX;
}

"??"        { step; saffire_push_state(st_regex); return T_COALESCE; }
"..."       { step; saffire_push_state(st_regex); return T_ELLIPSIS; }
".."        { step; saffire_push_state(st_regex); return T_TO; }
"::"        { step; saffire_push_state(st_regex); return T_NS_SEP; }
"~="        { step; saffire_push_state(st_regex); return T_RE; }
">="        { step; saffire_push_state(st_regex); return T_GE; }
"<="        { step; saffire_push_state(st_regex); return T_LE; }
"=="        { step; saffire_push_state(st_regex); return T_EQ; }
"!="        { step; saffire_push_state(st_regex); return T_NE; }
"in"        { step; saffire_push_state(st_regex); return T_IN; }

"||"        { step; saffire_push_state(st_regex); return T_OR; }
"&&"        { step; saffire_push_state(st_regex); return T_AND; }

"<<"        { step; saffire_push_state(st_regex); return T_SHIFT_LEFT; }
">>"        { step; saffire_push_state(st_regex); return T_SHIFT_RIGHT; }

"="         { step; saffire_push_state(st_regex); return T_ASSIGNMENT; }
"+="        { step; saffire_push_state(st_regex); return T_PLUS_ASSIGNMENT; }
"-="        { step; saffire_push_state(st_regex); return T_MINUS_ASSIGNMENT; }
"*="        { step; saffire_push_state(st_regex); return T_MUL_ASSIGNMENT; }
"/="        { step; saffire_push_state(st_regex); return T_DIV_ASSIGNMENT; }
"%="        { step; saffire_push_state(st_regex); return T_MOD_ASSIGNMENT; }
"&="        { step; saffire_push_state(st_regex); return T_AND_ASSIGNMENT; }
"|="        { step; saffire_push_state(st_regex); return T_OR_ASSIGNMENT; }
"^="        { step; saffire_push_state(st_regex); return T_XOR_ASSIGNMENT; }
"<<="       { step; saffire_push_state(st_regex); return T_SL_ASSIGNMENT; }
">>="       { step; saffire_push_state(st_regex); return T_SR_ASSIGNMENT; }

"++"        { step; saffire_push_state(st_div); return T_OP_INC; }
"--"        { step; saffire_push_state(st_div); return T_OP_DEC; }

"while"     { step; saffire_push_state(st_regex); return T_WHILE; }
"if"        { step; saffire_push_state(st_regex); return T_IF; }
"else"      { step; saffire_push_state(st_regex); return T_ELSE; }
"use"       { step; saffire_push_state(st_regex); return T_USE; }
"as"        { step; saffire_push_state(st_regex); return T_AS; }
"import"    { step; saffire_push_state(st_regex); return T_IMPORT; }
"from"      { step; saffire_push_state(st_regex); return T_FROM; }
"do"        { step; saffire_push_state(st_regex); return T_DO; }
"for"       { step; saffire_push_state(st_regex); return T_FOR; }
"foreach"   { step; saffire_push_state(st_regex); return T_FOREACH; }
"switch"    { step; saffire_push_state(st_regex); return T_SWITCH; }

"class"         { step; saffire_push_state(st_regex); return T_CLASS; }
"extends"       { step; saffire_push_state(st_regex); return T_EXTENDS; }
"implements"    { step; saffire_push_state(st_regex); return T_IMPLEMENTS; }
"inherits"      { step; saffire_push_state(st_regex); return T_INHERITS; }
"abstract"      { step; saffire_push_state(st_regex); return T_ABSTRACT; }
"final"         { step; saffire_push_state(st_regex); return T_FINAL; }
"interface"     { step; saffire_push_state(st_regex); return T_INTERFACE; }
"const"         { step; saffire_push_state(st_regex); return T_CONST; }
"static"        { step; saffire_push_state(st_regex); return T_STATIC; }

"public"        { step; saffire_push_state(st_regex); return T_PUBLIC; }
"private"       { step; saffire_push_state(st_regex); return T_PRIVATE; }
"protected"     { step; saffire_push_state(st_regex); return T_PROTECTED; }
"method"        { step; saffire_push_state(st_regex); return T_METHOD; }
"property"      { step; saffire_push_state(st_regex); return T_PROPERTY; }

"catch"         { step; saffire_push_state(st_regex); return T_CATCH; }
"finally"       { step; saffire_push_state(st_regex); return T_FINALLY; }
"throw"         { step; saffire_push_state(st_regex); return T_THROW; }
"return"        { step; saffire_push_state(st_regex); return T_RETURN; }
"break"         { step; saffire_push_state(st_regex); return T_BREAK; }
"breakelse"     { step; saffire_push_state(st_regex); return T_BREAKELSE; }
"continue"      { step; saffire_push_state(st_regex); return T_CONTINUE; }
"try"           { step; saffire_push_state(st_regex); return T_TRY; }
"default"       { step; saffire_push_state(st_regex); return T_DEFAULT; }
"goto"          { step; saffire_push_state(st_regex); return T_GOTO; }
"case"          { step; saffire_push_state(st_regex); return T_CASE; }

"self"          { step; saffire_push_state(st_regex); return T_SELF; }
"parent"        { step; saffire_push_state(st_regex); return T_PARENT; }

"yield"         { step; saffire_push_state(st_regex); return T_YIELD; }



<st_div,st_regex>\"         {
                                step;
                                /* Opening string quote */
                                saffire_push_state(st_string_dbl);
                                strbuf = NULL;
                            }
<st_div,st_regex>\'         {
                                step;
                                /* Opening string quote */
                                saffire_push_state(st_string_sgl);
                                strbuf = NULL;
                            }

<st_string_dbl>\"           {
                                step;
                                /* Closing double string quote */
                                yylval.sVal = smm_strdup(strbuf ? strbuf : "");
                                smm_free(strbuf);
                                saffire_pop_state();
                                return T_STRING;
                            }
<st_string_sgl>\'           {
                                step;
                                /* Closing Single string quote */
                                yylval.sVal = smm_strdup(strbuf ? strbuf : "");
                                smm_free(strbuf);
                                saffire_pop_state();
                                return T_STRING;
                            }

<st_string_sgl>\"           {
                                step;
                                /* Double quote is just a double quote in single quoted string */
                                strbuf = add_strbuf_char(strbuf, '\\'); }
<st_string_dbl>\'           {
                                step;
                                /* Single quote is just a single quote in double quoted string */
                                strbuf = add_strbuf_char(strbuf, '\''); }

<st_string_sgl,
 st_string_dbl>\n           {
                                step_nl;
                                /* Real newlines inside strings are not allowed */
                                error("newline found in string\n");
                            }
<st_string_sgl,
 st_string_dbl>\\\\         { strbuf = add_strbuf_char(strbuf, '\\'); }
<st_string_sgl,
 st_string_dbl>\\\"         { strbuf = add_strbuf_char(strbuf, '\"'); }
<st_string_sgl,
 st_string_dbl>\\\'         { step; strbuf = add_strbuf_char(strbuf, '\''); }

<st_string_sgl,
 st_string_dbl>\\0          { step; strbuf = add_strbuf_char(strbuf, '\0'); }
<st_string_sgl,
 st_string_dbl>\\[0-7]{1,3} {
                                step;

                                /* Octal number */
                                int result;
                                (void) sscanf( yytext + 1, "%o", &result);
                                if (result > 0xff) {
                                    error("octal value '%s' is out of bounds\n", yytext);
                                    yyterminate();
                                }
                                strbuf = add_strbuf_char(strbuf, (char)result);
                            }
<st_string_sgl,
 st_string_dbl>\\[xX][0-9A-Fa-f]{1,2} {
                                step;

                                /* Hexidecimal number */
                                int result;
                                (void) sscanf(yytext + 2, "%X", &result);
                                if (result > 0xff) {
                                    error("hex value '%s' is out of bounds\n", yytext);
                                    yyterminate();
                                }
                                strbuf = add_strbuf_char(strbuf, (char)result);

                            }

<st_string_sgl,
 st_string_dbl>\\[nN]           { step; strbuf = add_strbuf_char(strbuf, '\n'); }
<st_string_sgl,
 st_string_dbl>\\[tT]           { step; strbuf = add_strbuf_char(strbuf, '\t'); }
<st_string_sgl,
 st_string_dbl>\\[bB]           { step; strbuf = add_strbuf_char(strbuf, '\b'); }
<st_string_sgl,
 st_string_dbl>\\[rR]           { step; strbuf = add_strbuf_char(strbuf, '\r'); }
<st_string_sgl,
 st_string_dbl>\\[fF]           { step; strbuf = add_strbuf_char(strbuf, '\f'); }
<st_string_sgl,
  st_string_dbl>\\[vV]          { step; strbuf = add_strbuf_char(strbuf, '\v'); }
<st_string_sgl,
 st_string_dbl>\\.              {
                                step;
                                /* Everything else is an unknown escape string */
                                error("unknown escape '%s' in string\n", yytext);
                                yyterminate();
                            }
<st_string_dbl>[^\\"\n]*        { step; strbuf = add_strbuf(strbuf, yytext); }
<st_string_sgl>[^\\'\n]*        { step; strbuf = add_strbuf(strbuf, yytext); }


{identifier} {
    step;
    saffire_push_state(st_div);
    yylval.sVal = smm_strdup(yytext);
    return T_IDENTIFIER;
}

{lnum} {
    step;
    saffire_push_state(st_div);
    yylval.lVal = strtol(yytext, NULL, 10);
    return T_LNUM;
}

. {
    step;
    error("Unknown token: '%s'\n", yytext);
    yyterminate();
}

%%

int yywrap(void) { return 1; }
