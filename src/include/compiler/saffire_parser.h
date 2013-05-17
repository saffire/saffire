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
#ifndef __COMPILER_SAFFIRE_PARSER_H__
#define __COMPILER_SAFFIRE_PARSER_H__

    #include "compiler/ast_nodes.h"

// @TODO: VM running mode into this structure

    #define SAFFIRE_EXECMODE_REPL       1
    #define SAFFIRE_EXECMODE_FILE       2

    typedef struct SaffireParser SaffireParser;

    struct SaffireParser {
        int     mode;                       // SAFFIRE_EXECMODE_* constants

        FILE    *filehandle;                // FILE to read from (or NULL)

        int     (*yyparse)(void *, int, char *, int);    // actual yyparse function, or NULL to use the default one
        void    *yyparse_args;              // Pointer to argument structure. Actual context known to the called yyparse method

        void    (*yyexec)(t_ast_element *, SaffireParser *);

        t_ast_element  *ast;                // Generated AST element

        char    *error;                     // Returned error
        int     eof;                        // End of file reached, or user quits repl
    };

#endif
