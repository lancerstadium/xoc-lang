/**
 * @file    xoc_compiler.h
 * @brief   XOC Compiler Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC Compiler.
 * @note    Use standard bare C headers for cross-compatibility.
 */

#ifndef XOC_COMPILER_H
#define XOC_COMPILER_H

#include "xoc_common.h"
#include "xoc_types.h"
#include "xoc_lexer.h"
#include "xoc_parser.h"

struct xoc_compiler_option {
    int argc;
    char** argv;
    bool is_filesys_enabled;
    bool is_impllib_enabled;
};

struct xoc_compiler {
    pool_t      syms;   // string symbols
    pool_t      blks;   // insn blocks
    mods_t      mods;
    map_t       sym_tbl; // symbol table

    lexer_t     lex;
    parser_t    prs;

    compiler_option_t opt;
    info_t      info;
    log_t       log;
};

void compiler_init(compiler_t* cp, const char* file, const char* src, compiler_option_t* opt);
void compiler_free(compiler_t* cp);



#endif /* XOC_COMPILER_H */