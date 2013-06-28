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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "compiler/output/asm.h"
#include "general/output.h"
#include "general/smm.h"
#include "general/dll.h"
#include "vm/vm_opcodes.h"
#include "debug.h"

/**
 * Converts assembler codes into bytecode
 */

struct _backpatch {
    long opcode_offset;             // Points to the opcode that we are actually patching
    long operand_offset;            // Points to the operand that we need to patch
    char *label;
};

/**
 * Calculate the maximum stack size needed for this frame (run all available paths)
 */
static int _calculate_maximum_stack_size(t_asm_frame *frame) {
    // @TODO: Fixed now, must be calculated!
    return 42;
}


/**
 * Backpatch label offsets for this frame.
 */
static void _backpatch_labels(t_asm_frame *frame) {
    t_dll_element *e = DLL_HEAD(frame->backpatch_offsets);
    int start_offset;

    while (e) {
        struct _backpatch *bp = (struct _backpatch *)e->data;

        // Check if labelname exists, if not, we have referenced to a label but never declared it.
        if (! ht_exists(frame->label_offsets, bp->label)) {
            char *label = bp->label;
            label += strlen("userlabel_");
            fatal_error(1, "Cannot find label '%s'\n", label);
        }
        // Fetch the offset of the label so we can patch it
        unsigned int label_offset = (int)ht_find(frame->label_offsets, bp->label);

        // We need to add an offset to our patching. This is because relative items are calculated from the LAST
        // operand read. The current operand does not have to be the last one for this opcode.
        start_offset = bp->opcode_offset + 1;    // @TODO: Multibyte opcodes should be handled as well!
        if (((unsigned char)frame->code[bp->opcode_offset] & 0x80) == 0x80) start_offset += 2;
        if (((unsigned char)frame->code[bp->opcode_offset] & 0xC0) == 0xC0) start_offset += 2;
        if (((unsigned char)frame->code[bp->opcode_offset] & 0xE0) == 0xE0) start_offset += 2;

        // Check if it needs to be absolute or relative
        if ((unsigned char)frame->code[bp->opcode_offset] != VM_BUILD_CLASS &&
            (unsigned char)frame->code[bp->opcode_offset] != VM_CONTINUE_LOOP &&
            (unsigned char)frame->code[bp->opcode_offset] != VM_JUMP_ABSOLUTE) {
            // Convert absolute offset to a relative offset
            label_offset -= start_offset;
        }

        // Patch it!
        uint16_t *ptr = (uint16_t *)(frame->code + bp->operand_offset);
        *ptr = (label_offset & 0xFFFF);

        e = DLL_NEXT(e);
    }
}


/**
 * Add a byte to the codespace (takes care of needed memory allocations)
 */
void _add_codebyte(t_asm_frame *frame, unsigned char b) {
    // Check if we need to (re)allocate space for bytecode
    if (frame->code_len >= frame->alloc_len) {
        frame->alloc_len += 1024;       // Add additional kilobyte of codespace
        frame->code = smm_realloc(frame->code, frame->alloc_len);
    }
    frame->code[frame->code_len] = b;
    frame->code_len++;
}


/**
 * Find the constant in our constant table and return it's constant offset. When
 * not found, just add it's offset.
 */
static int _convert_constant_string(t_asm_frame *frame, char *s) {
    int pos = 0;
    t_dll_element *e = DLL_HEAD(frame->constants);
    while (e) {
        t_asm_constant *c = (t_asm_constant *)e->data;

        if (c->type == const_string && strcmp(s, c->data.s) == 0) return pos;
        pos++;
        e = DLL_NEXT(e);
    }

    // Add to DLL
    t_asm_constant *c = smm_malloc(sizeof(t_asm_constant));
    c->type = const_string;
    c->data.s = smm_strdup(s);
    dll_append(frame->constants, c);

    return frame->constants->size - 1;
}
static int _convert_constant_code(t_asm_frame *frame, char *s) {
    int pos = 0;
    t_dll_element *e = DLL_HEAD(frame->constants);
    while (e) {
        t_asm_constant *c = (t_asm_constant *)e->data;

        if (c->type == const_code && strcmp(s, c->data.s) == 0) return pos;
        pos++;
        e = DLL_NEXT(e);
    }

    // Add to DLL
    t_asm_constant *c = smm_malloc(sizeof(t_asm_constant));
    c->type = const_code;
    c->data.s = smm_strdup(s);
    dll_append(frame->constants, c);

    return frame->constants->size - 1;
}

static int _convert_constant_numerical(t_asm_frame *frame, int i) {
    int pos = 0;

    t_dll_element *e = DLL_HEAD(frame->constants);
    while (e) {
        t_asm_constant *c = (t_asm_constant *)e->data;

        if (c->type == const_long && c->data.l == i) return pos;
        pos++;
        e = DLL_NEXT(e);
    }

    // Add to DLL
    t_asm_constant *c = smm_malloc(sizeof(t_asm_constant));
    c->type = const_long;
    c->data.l = i;
    dll_append(frame->constants, c);
    return frame->constants->size - 1;
}
static int _convert_identifier(t_asm_frame *frame, char *s) {
    int pos = 0;

    t_dll_element *e = DLL_HEAD(frame->identifiers);
    while (e) {
        char *t = (char *)e->data;
        if (strcmp(t, s) == 0) return pos;
        pos++;
        e = DLL_NEXT(e);
    }
    // Add to DLL
    dll_append(frame->identifiers, s);
    return frame->identifiers->size - 1;
}

/**
 * Creates a single assembler frame. Stops when end of lines, or when a new frame is encountered
 */
static t_asm_frame *assemble_frame(t_dll *source_frame) {
    t_asm_line *line;
    struct _backpatch *bp;
    int opr = VM_STOP;

    // Create frame
    t_asm_frame *frame = smm_malloc(sizeof(t_asm_frame));
    frame->constants = dll_init();
    frame->identifiers = dll_init();
    frame->label_offsets = ht_create();             // key: label_name => value: offset
    frame->backpatch_offsets = dll_init();
    frame->alloc_len = 0;
    frame->code_len = 0;
    frame->code = NULL;
    frame->lino_len = 0;
    frame->lino = NULL;

    t_dll *tc = dll_init();
    int old_lineno = 0;
    int old_opcode_off = 0;

    t_dll_element *e = DLL_HEAD(source_frame);
    while (e) {
        line = (t_asm_line *)e->data;

        if (line->type == ASM_LINE_TYPE_LABEL) {
            // Found a label. Store it so we can backpatch it later
            if (ht_exists(frame->label_offsets, line->s)) {
                fatal_error(1, "Label '%s' is already defined", line->s);
            }
            ht_add(frame->label_offsets, line->s, (void *)frame->code_len);
        }

        if (line->type == ASM_LINE_TYPE_CODE) {
            // Regular opcode found

            // Save opcode offset. Normally opcodes are one byte, but we reserve future use for 2 or more.
            int opcode_off = frame->code_len;
            _add_codebyte(frame, line->opcode);

            if (line->lineno != 0 && line->lineno != old_lineno) {
                int delta_lineno = line->lineno - old_lineno;
                int delta_codeoff = opcode_off - old_opcode_off;
                old_opcode_off = opcode_off;
                old_lineno = line->lineno;

                do {
                    if (delta_codeoff > 127) {
                        dll_append(tc, (void *)(128 | 127));
                        delta_codeoff -= 127;
                    } else {
                        dll_append(tc, (void *)delta_codeoff);
                        delta_codeoff = 0;
                    }
                } while (delta_codeoff != 0);

                do {
                    if (delta_lineno > 127) {
                        dll_append(tc, (void *)(128 | 127));
                        delta_lineno -= 127;
                    } else {
                        dll_append(tc, (void *)delta_lineno);
                        delta_lineno = 0;
                    }
                } while (delta_lineno != 0);
            }


            for (int i=0; i!=line->opr_count; i++) {
                switch (line->opr[i]->type) {
                    case ASM_LINE_TYPE_OP_LABEL :
                        bp = smm_malloc(sizeof(struct _backpatch));
                        bp->opcode_offset = opcode_off;
                        bp->operand_offset = frame->code_len;
                        bp->label = smm_strdup(line->opr[i]->data.s);
                        dll_append(frame->backpatch_offsets, bp);

                        opr = 0xFFFF; // Add dummy bytes keep the offsets happy
                        break;
                    case ASM_LINE_TYPE_OP_STRING :
                        opr = _convert_constant_string(frame, line->opr[i]->data.s);
                        break;
                    case ASM_LINE_TYPE_OP_CODE :
                        opr = _convert_constant_code(frame, line->opr[i]->data.s);
                        break;
                    case ASM_LINE_TYPE_OP_NUM :
                        opr = _convert_constant_numerical(frame, line->opr[i]->data.l);
                        break;
                    case ASM_LINE_TYPE_OP_REALNUM :
                    case ASM_LINE_TYPE_OP_COMPARE :
                    case ASM_LINE_TYPE_OP_OPERATOR :
                        opr = line->opr[i]->data.l;
                        break;

                    case ASM_LINE_TYPE_OP_ID :
                        opr = _convert_identifier(frame, line->opr[i]->data.s);
                        break;
                }

                _add_codebyte(frame, (unsigned char)(opr & 0x00FF));           // Add lo 8 bits
                _add_codebyte(frame, (unsigned char)((opr & 0xFF00) >> 8));     // Add hi 8 bits
            }
        }

        e = DLL_NEXT(e);
    }

    // Backpatch labels
    _backpatch_labels(frame);

    // Calculate maximum stack frame
    frame->stack_size = _calculate_maximum_stack_size(frame);


    // Set our linenumbers
    frame->lino_len = tc->size;
    frame->lino = (char *)smm_malloc(frame->lino_len);

    int j = 0;
    if (tc->size > 0) {
        e = DLL_HEAD(tc)->next;     // Skip first element.
        dll_append(tc, (void *)0);  // Add trailing marker
        while (e) {
            int i = (int)e->data;
            frame->lino[j++] = (unsigned char)(i & 0xFF);
            e = DLL_NEXT(e);
        }
    }

    return frame;
}


/**
 * Free an codeline operator
 */
static void _asm_free_opr(t_asm_opr *opr) {
    if (opr->type == ASM_LINE_TYPE_OP_LABEL ||
        opr->type == ASM_LINE_TYPE_OP_STRING ||
        opr->type == ASM_LINE_TYPE_OP_ID) {
        smm_free(opr->data.s);
    }
    smm_free(opr);
}

/**
 * Free an assembler line
 */
static void _asm_free_line(t_asm_line *line) {
    switch (line->type) {
        case ASM_LINE_TYPE_LABEL :
            smm_free(line->s);
            break;
        case ASM_LINE_TYPE_CODE :
            for (int i=0; i!=line->opr_count; i++) {
                 _asm_free_opr(line->opr[i]);
            }
            break;
    }
    smm_free(line);
}

/**
 * Free assembler DLL structure
 */
void assembler_free(t_hash_table *asm_code) {
    t_hash_iter iter;
    ht_iter_init(&iter, asm_code);

    while (ht_iter_valid(&iter)) {
        t_dll *frame = ht_iter_value(&iter);

        t_dll_element *e = DLL_HEAD(frame);
        while (e) {
            _asm_free_line((t_asm_line *)e->data);
            e = DLL_NEXT(e);
        }

        dll_free(frame);

        ht_iter_next(&iter);
    }

    ht_destroy(asm_code);
}


/**
 * Create an operand that can be added to a codeline
 */
t_asm_opr *asm_create_opr(int type, char *s, int l) {
    t_asm_opr *opr = smm_malloc(sizeof(t_asm_opr));
    opr->type = type;
    if (s != NULL) {
        opr->data.s = smm_strdup(s);
    } else {
        opr->data.l = l;
    }
    return opr;
}

/**
 * Create a code line, with 0 or more operands
 */
t_asm_line *asm_create_codeline(int lineno, int opcode, int opr_cnt, ...) {
    t_asm_line *line = smm_malloc(sizeof(t_asm_line));
    line->type = ASM_LINE_TYPE_CODE;
    line->opcode = opcode;
    line->opr_count = opr_cnt;
    line->opr = smm_malloc(sizeof(t_asm_opr *) * opr_cnt);
    line->lineno = lineno;

    va_list oprs;
    va_start(oprs, opr_cnt);
    for (int i=0; i!=opr_cnt; i++) {
        t_asm_opr *opr = va_arg(oprs, t_asm_opr *);
        line->opr[i] = opr;
    }
    va_end(oprs);
    return line;
}


/**
 * Create a label line
 */
t_asm_line *asm_create_labelline(char *label) {
    t_asm_line *line = smm_malloc(sizeof(t_asm_line));
    line->type = ASM_LINE_TYPE_LABEL;
    line->s = smm_strdup(label);
    return line;
}



/**
 *
 */
t_bytecode *assembler(t_hash_table *asm_code, const char *filename) {
    t_hash_iter iter;

    // Init
    t_hash_table *assembled_frames = ht_create();

    ht_iter_init(&iter, asm_code);
    while (ht_iter_valid(&iter)) {
        t_dll *frame = ht_iter_value(&iter);
        char *key = ht_iter_key(&iter);

        t_asm_frame *assembled_frame = assemble_frame(frame);
        ht_add(assembled_frames, key, assembled_frame);

        ht_iter_next(&iter);
    }

    t_bytecode *bc = convert_frames_to_bytecode(assembled_frames, "main", 1);
    bc->source_filename = filename ? strdup(filename) : NULL;
    ht_destroy(assembled_frames);
    return bc;
}


static int last_lineno = 0;

/**
 * Ouput a complete frame
 */
static void _assembler_output_frame(t_dll *frame, FILE *f) {
    int oprcnt = 0;
    t_dll_element *e = DLL_HEAD(frame);
    while (e) {
        t_asm_line *line = (t_asm_line *)e->data;

        if (line->type == ASM_LINE_TYPE_LABEL) {
            fprintf(f, "#%s:", line->s);
        }
        if (line->type == ASM_LINE_TYPE_CODE) {
            if (last_lineno != line->lineno && line->lineno) {
                if (line->lineno != 1) fprintf(f, "\n");
                fprintf(f, "% 5d     ", line->lineno);
                last_lineno = line->lineno;
            } else {
                fprintf(f, "          ");
            }

            fprintf(f, "%5d %-20s", oprcnt, vm_code_names[vm_codes_offset[line->opcode]]);
            oprcnt++;

            // Output additional operands
            for (int i=0; i!=line->opr_count; i++) {
                switch (line->opr[i]->type) {
                    case ASM_LINE_TYPE_OP_LABEL :
                        fprintf(f, "#%s", line->opr[i]->data.s);
                        break;
                    case ASM_LINE_TYPE_OP_STRING :
                        fprintf(f, "\"%s\"", line->opr[i]->data.s);
                        break;
                    case ASM_LINE_TYPE_OP_CODE :
                        fprintf(f, "@%s", line->opr[i]->data.s);
                        break;
                    case ASM_LINE_TYPE_OP_REALNUM :
                        fprintf(f, "$%ld", line->opr[i]->data.l);
                        break;
                    case ASM_LINE_TYPE_OP_NUM :
                        fprintf(f, "%ld", line->opr[i]->data.l);
                        break;
                    case ASM_LINE_TYPE_OP_OPERATOR :
                        switch (line->opr[i]->data.l) {
                            case OPERATOR_ADD : fprintf(f, "ADD"); break;
                            case OPERATOR_SUB : fprintf(f, "SUB"); break;
                            case OPERATOR_MUL : fprintf(f, "MUL"); break;
                            case OPERATOR_DIV : fprintf(f, "DIV"); break;
                            case OPERATOR_MOD : fprintf(f, "MOD"); break;
                            case OPERATOR_AND : fprintf(f, "AND"); break;
                            case OPERATOR_OR  : fprintf(f, "OR");  break;
                            case OPERATOR_XOR : fprintf(f, "XOR"); break;
                            case OPERATOR_SHL : fprintf(f, "SHL"); break;
                            case OPERATOR_SHR : fprintf(f, "SHR"); break;
                            case OPERATOR_COA : fprintf(f, "COA"); break;
                        }
                        break;
                    case ASM_LINE_TYPE_OP_COMPARE :
                        switch (line->opr[i]->data.l) {
                            case COMPARISON_EQ : fprintf(f, "EQ"); break;
                            case COMPARISON_NE : fprintf(f, "NE"); break;
                            case COMPARISON_LT : fprintf(f, "LT"); break;
                            case COMPARISON_LE : fprintf(f, "LE"); break;
                            case COMPARISON_GT : fprintf(f, "GT"); break;
                            case COMPARISON_GE : fprintf(f, "GE"); break;
                            case COMPARISON_IN : fprintf(f, "IN"); break;
                            case COMPARISON_NI : fprintf(f, "NI"); break;
                            case COMPARISON_EX : fprintf(f, "EX"); break;
                        }
                        break;
                    case ASM_LINE_TYPE_OP_ID :
                        fprintf(f, "%s", line->opr[i]->data.s);
                        break;
                }
                if (i < line->opr_count-1) {
                    fprintf(f, ", ");
                }
                oprcnt+=2;
            }
        }
        fprintf(f, "\n");

        e = DLL_NEXT(e);
    }
}

/**
 * Outputs a textual assembler file
 */
void assembler_output(t_hash_table *asm_code, char *output_path) {
    FILE *f = fopen(output_path, "w");
    if (! f) return;

    assembler_output_stream(asm_code, f);

    fclose(f);
}

void assembler_output_stream(t_hash_table *asm_code, FILE *f) {
    t_hash_iter iter;

    ht_iter_init(&iter, asm_code);
    while (ht_iter_valid(&iter)) {
        t_dll *frame = ht_iter_value(&iter);
        char *key = ht_iter_key(&iter);

        // The "main frame does not need any declaration. Other frames do:
        if (strcmp(key, "main") != 0) {
            fprintf(f, "\n\n@%s:\n", key);
        }

        _assembler_output_frame(frame, f);

        ht_iter_next(&iter);
    }
}
