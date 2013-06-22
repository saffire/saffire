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
    #include <ctype.h>
    #include "repl/repl.h"

    #include "general/output.h"
    #include "compiler/parser_helpers.h"
    #include "general/smm.h"
    #include "compiler/ast_nodes.h"
    #include "compiler/parser.tab.h"
    #include "compiler/saffire_parser.h"

    // Forward defines
    size_t yyread(FILE *f, char *buf, size_t max, SaffireParser *sp, int lineno);
    static void yy_push_state(int, yyscan_t);
    static void yy_pop_state(yyscan_t);

    extern int yylex(union YYSTYPE * yylval, YYLTYPE *yylloc, yyscan_t scanner);


    // yylex() takes an additional argument
    #define YY_EXTRA_TYPE SaffireParser *

    // Redefine the YY_INPUT macro so we decide on how to read characters
    #undef YY_INPUT
    #define YY_INPUT(buf, result, max_size) result = yyread(yyin, buf, max_size, yyextra, yylineno)


    // Set the useraction macro. Make sure that when we have typed something, we are not atStart anymore (so we can display the ps2 prompt)
    #define YY_USER_ACTION  if (!isspace(*yytext)) { repl_argstruct_t *args = yyextra->yyparse_args; if (args) args->atStart = 0;  }


    #define saffire_push_state(state) yy_push_state(state, yyscanner)
    #define saffire_pop_state() yy_pop_state(yyscanner)
    #define saffire_top_state() yy_top_state(yyscanner)



    // These two defines take care of line/column maintenance
    #define do_yylloc                                           \
        do {                                                    \
            yylloc->first_column   = yylloc->last_column + 1;     \
            yylloc->last_column += strlen(yytext);               \
        } while(0)

    #define do_yylloc_nl                                             \
        do {                                                    \
            yylloc->first_line = yylloc->last_line = yylineno;    \
            yylloc->first_column = 1;                            \
            yylloc->last_column = 0;                             \
        } while (0)


    static char *strbuf;


    // Helpers for creating quoted string buffers (both single and double quoted strings)
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

%}

/* Define lexer states */
%s st_regex
%s st_div

/* Important: exclusive states! Matches no other lexemes (like identifier, whitespace etc) */
%x st_string_dbl
%x st_string_sgl


/* Using reentrant parser */
%option reentrant
%option bison-bridge
%option bison-locations
%option extra-type="SaffireParser *"

/* Add linenumbering */
%option yylineno

/* No need for yywrap */
%option noyywrap

/* Add debug info */
%option debug

/* We're using start condition stacks */
%option stack

/* We define our own yymalloc functionality */
%option noyyfree noyyrealloc noyyalloc

/* suppress -Wall warning about yyunput */
%option nounput


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
{newline}                   { do_yylloc_nl; }

<st_div>\/ { return '/'; }

[-+\%\/<>\(\)\{\}:;,\.\[\]\?!\*^\|]    { do_yylloc; saffire_push_state(st_regex); return yytext[0]; }


    /* Only match regex when we are in the regex state */
<st_regex>\/[^\/\\]*(?:\\.[^\/\\]*)*\/[a-z]*    {
    do_yylloc;
    yylval->sVal = smm_strdup(yytext);
    return T_REGEX;
}

"??"        { do_yylloc; saffire_push_state(st_regex); return T_COALESCE; }
"..."       { do_yylloc; saffire_push_state(st_regex); return T_ELLIPSIS; }
".."        { do_yylloc; saffire_push_state(st_regex); return T_TO; }
"::"        { do_yylloc; saffire_push_state(st_regex); return T_NS_SEP; }
"~="        { do_yylloc; saffire_push_state(st_regex); return T_RE; }
">="        { do_yylloc; saffire_push_state(st_regex); return T_GE; }
"<="        { do_yylloc; saffire_push_state(st_regex); return T_LE; }
"=="        { do_yylloc; saffire_push_state(st_regex); return T_EQ; }
"!="        { do_yylloc; saffire_push_state(st_regex); return T_NE; }
"in"        { do_yylloc; saffire_push_state(st_regex); return T_IN; }

"||"        { do_yylloc; saffire_push_state(st_regex); return T_OR; }
"&&"        { do_yylloc; saffire_push_state(st_regex); return T_AND; }

"<<"        { do_yylloc; saffire_push_state(st_regex); return T_SHIFT_LEFT; }
">>"        { do_yylloc; saffire_push_state(st_regex); return T_SHIFT_RIGHT; }

"="         { do_yylloc; saffire_push_state(st_regex); return T_ASSIGNMENT; }
"+="        { do_yylloc; saffire_push_state(st_regex); return T_PLUS_ASSIGNMENT; }
"-="        { do_yylloc; saffire_push_state(st_regex); return T_MINUS_ASSIGNMENT; }
"*="        { do_yylloc; saffire_push_state(st_regex); return T_MUL_ASSIGNMENT; }
"/="        { do_yylloc; saffire_push_state(st_regex); return T_DIV_ASSIGNMENT; }
"%="        { do_yylloc; saffire_push_state(st_regex); return T_MOD_ASSIGNMENT; }
"&="        { do_yylloc; saffire_push_state(st_regex); return T_AND_ASSIGNMENT; }
"|="        { do_yylloc; saffire_push_state(st_regex); return T_OR_ASSIGNMENT; }
"^="        { do_yylloc; saffire_push_state(st_regex); return T_XOR_ASSIGNMENT; }
"<<="       { do_yylloc; saffire_push_state(st_regex); return T_SL_ASSIGNMENT; }
">>="       { do_yylloc; saffire_push_state(st_regex); return T_SR_ASSIGNMENT; }

"++"        { do_yylloc; saffire_push_state(st_div); return T_OP_INC; }
"--"        { do_yylloc; saffire_push_state(st_div); return T_OP_DEC; }

"while"     { do_yylloc; saffire_push_state(st_regex); return T_WHILE; }
"if"        { do_yylloc; saffire_push_state(st_regex); return T_IF; }
"else"      { do_yylloc; saffire_push_state(st_regex); return T_ELSE; }
"use"       { do_yylloc; saffire_push_state(st_regex); return T_USE; }
"as"        { do_yylloc; saffire_push_state(st_regex); return T_AS; }
"import"    { do_yylloc; saffire_push_state(st_regex); return T_IMPORT; }
"from"      { do_yylloc; saffire_push_state(st_regex); return T_FROM; }
"do"        { do_yylloc; saffire_push_state(st_regex); return T_DO; }
"for"       { do_yylloc; saffire_push_state(st_regex); return T_FOR; }
"foreach"   { do_yylloc; saffire_push_state(st_regex); return T_FOREACH; }
"switch"    { do_yylloc; saffire_push_state(st_regex); return T_SWITCH; }

"class"         { do_yylloc; saffire_push_state(st_regex); return T_CLASS; }
"extends"       { do_yylloc; saffire_push_state(st_regex); return T_EXTENDS; }
"implements"    { do_yylloc; saffire_push_state(st_regex); return T_IMPLEMENTS; }
"inherits"      { do_yylloc; saffire_push_state(st_regex); return T_INHERITS; }
"abstract"      { do_yylloc; saffire_push_state(st_regex); return T_ABSTRACT; }
"final"         { do_yylloc; saffire_push_state(st_regex); return T_FINAL; }
"interface"     { do_yylloc; saffire_push_state(st_regex); return T_INTERFACE; }
"const"         { do_yylloc; saffire_push_state(st_regex); return T_CONST; }
"static"        { do_yylloc; saffire_push_state(st_regex); return T_STATIC; }

"public"        { do_yylloc; saffire_push_state(st_regex); return T_PUBLIC; }
"private"       { do_yylloc; saffire_push_state(st_regex); return T_PRIVATE; }
"protected"     { do_yylloc; saffire_push_state(st_regex); return T_PROTECTED; }
"method"        { do_yylloc; saffire_push_state(st_regex); return T_METHOD; }
"property"      { do_yylloc; saffire_push_state(st_regex); return T_PROPERTY; }

"catch"         { do_yylloc; saffire_push_state(st_regex); return T_CATCH; }
"finally"       { do_yylloc; saffire_push_state(st_regex); return T_FINALLY; }
"throw"         { do_yylloc; saffire_push_state(st_regex); return T_THROW; }
"return"        { do_yylloc; saffire_push_state(st_regex); return T_RETURN; }
"break"         { do_yylloc; saffire_push_state(st_regex); return T_BREAK; }
"breakelse"     { do_yylloc; saffire_push_state(st_regex); return T_BREAKELSE; }
"continue"      { do_yylloc; saffire_push_state(st_regex); return T_CONTINUE; }
"try"           { do_yylloc; saffire_push_state(st_regex); return T_TRY; }
"default"       { do_yylloc; saffire_push_state(st_regex); return T_DEFAULT; }
"goto"          { do_yylloc; saffire_push_state(st_regex); return T_GOTO; }
"case"          { do_yylloc; saffire_push_state(st_regex); return T_CASE; }

"self"          { do_yylloc; saffire_push_state(st_regex); return T_SELF; }
"parent"        { do_yylloc; saffire_push_state(st_regex); return T_PARENT; }

"yield"         { do_yylloc; saffire_push_state(st_regex); return T_YIELD; }



<st_div,st_regex>\"         {
                                do_yylloc;
                                /* Opening string quote */
                                saffire_push_state(st_string_dbl);
                                strbuf = NULL;
                            }
<st_div,st_regex>\'         {
                                do_yylloc;
                                /* Opening string quote */
                                saffire_push_state(st_string_sgl);
                                strbuf = NULL;
                            }

<st_string_dbl>\"           {
                                do_yylloc;
                                /* Closing double string quote */
                                yylval->sVal = smm_strdup(strbuf ? strbuf : "");
                                smm_free(strbuf);
                                saffire_pop_state();
                                return T_STRING;
                            }
<st_string_sgl>\'           {
                                do_yylloc;
                                /* Closing Single string quote */
                                yylval->sVal = smm_strdup(strbuf ? strbuf : "");
                                smm_free(strbuf);
                                saffire_pop_state();
                                return T_STRING;
                            }

<st_string_sgl>\"           {
                                do_yylloc;
                                /* Double quote is just a double quote in single quoted string */
                                strbuf = add_strbuf_char(strbuf, '\\'); }
<st_string_dbl>\'           {
                                do_yylloc;
                                /* Single quote is just a single quote in double quoted string */
                                strbuf = add_strbuf_char(strbuf, '\''); }

<st_string_sgl,
 st_string_dbl>\n           {
                                do_yylloc_nl;
                                /* Real newlines inside strings are not allowed */
                                parser_error((SaffireParser *)yyextra, yylloc->first_line, "newline found in string\n");
                            }
<st_string_sgl,
 st_string_dbl>\\\\         { strbuf = add_strbuf_char(strbuf, '\\'); }
<st_string_sgl,
 st_string_dbl>\\\"         { strbuf = add_strbuf_char(strbuf, '\"'); }
<st_string_sgl,
 st_string_dbl>\\\'         { do_yylloc; strbuf = add_strbuf_char(strbuf, '\''); }

<st_string_sgl,
 st_string_dbl>\\0          { do_yylloc; strbuf = add_strbuf_char(strbuf, '\0'); }
<st_string_sgl,
 st_string_dbl>\\[0-7]{1,3} {
                                do_yylloc;

                                /* Octal number */
                                int result;
                                (void) sscanf( yytext + 1, "%o", &result);
                                if (result > 0xff) {
                                    parser_error((SaffireParser *)yyextra, yylloc->first_line, "octal value '%s' is out of bounds\n", yytext);
                                    yyterminate();
                                }
                                strbuf = add_strbuf_char(strbuf, (char)result);
                            }
<st_string_sgl,
 st_string_dbl>\\[xX][0-9A-Fa-f]{1,2} {
                                do_yylloc;

                                /* Hexidecimal number */
                                int result;
                                (void) sscanf(yytext + 2, "%X", &result);
                                if (result > 0xff) {
                                    parser_error((SaffireParser *)yyextra, yylloc->first_line, "hex value '%s' is out of bounds\n", yytext);
                                    yyterminate();
                                }
                                strbuf = add_strbuf_char(strbuf, (char)result);

                            }

<st_string_sgl,
 st_string_dbl>\\[nN]           { do_yylloc; strbuf = add_strbuf_char(strbuf, '\n'); }
<st_string_sgl,
 st_string_dbl>\\[tT]           { do_yylloc; strbuf = add_strbuf_char(strbuf, '\t'); }
<st_string_sgl,
 st_string_dbl>\\[bB]           { do_yylloc; strbuf = add_strbuf_char(strbuf, '\b'); }
<st_string_sgl,
 st_string_dbl>\\[rR]           { do_yylloc; strbuf = add_strbuf_char(strbuf, '\r'); }
<st_string_sgl,
 st_string_dbl>\\[fF]           { do_yylloc; strbuf = add_strbuf_char(strbuf, '\f'); }
<st_string_sgl,
  st_string_dbl>\\[vV]          { do_yylloc; strbuf = add_strbuf_char(strbuf, '\v'); }
<st_string_sgl,
 st_string_dbl>\\.              {
                                do_yylloc;
                                /* Everything else is an unknown escape string */
                                parser_error((SaffireParser *)yyextra, yylloc->first_line, "unknown escape '%s' in string\n", yytext);
                                yyterminate();
                            }
<st_string_dbl>[^\\"\n]*        { do_yylloc; strbuf = add_strbuf(strbuf, yytext); }
<st_string_sgl>[^\\'\n]*        { do_yylloc; strbuf = add_strbuf(strbuf, yytext); }


{identifier} {
    do_yylloc;
    saffire_push_state(st_div);
    yylval->sVal = smm_strdup(yytext);
    return T_IDENTIFIER;
}

{lnum} {
    do_yylloc;
    saffire_push_state(st_div);
    yylval->lVal = strtol(yytext, NULL, 10);
    return T_LNUM;
}

. {
    do_yylloc;
    parser_error((SaffireParser *)yyextra, yylloc->first_line, "Unknown token: '%s'\n", yytext);
    yyterminate();
}

%%

/**
 * Flushes the rest of the look-ahead buffer. Used for restarting after a syntax error in REPL mode.
 */
void flush_buffer(yyscan_t *yyscanner) {
    struct yyguts_t *yyg = (struct yyguts_t *)yyscanner;
    YY_FLUSH_BUFFER;
}



/**
 * This is the main character read function (as defined through YY_INPUT). Depending on saffire execution
 * mode it will read from file, or display prompts and fetches data.
 */
size_t yyread(FILE *f, char *buf, size_t max, SaffireParser *sp, int lineno) {
    // Call our custom yyparse function if we have defined one
    if (
        sp->yyparse != NULL) {
        return sp->yyparse(sp->yyparse_args, lineno, buf, max);
    }

    // Read (part of) file
    size_t len = fread(buf, 1, max, sp->file);

    if (len == 0 && ferror(sp->file)) {
        sp->error = strdup("Error while reading file");
    }

    if (len == 0) {
        sp->eof = 1;
        return YY_NULL;
    }

    return len;
}

