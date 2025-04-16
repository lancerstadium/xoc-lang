/**
 * @file    xoc_lexer.h
 * @brief   XOC Lexer Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC Lexer.
 * @note    Use standard bare C headers for cross-compatibility.
 */

#ifndef XOC_LEXER_H
#define XOC_LEXER_H

#include "xoc_common.h"

struct xoc_token {
    tokenkind_t kind;                                   /** Token kind */
    int row;                                            /** Line number */
    int pos;                                            /** Position in row */
    union {
        struct {
            // identname_t name;                           /** Identifier name */
            unsigned int key;                           /** Identifier hash key */
        };
        int64_t     Int;
        uint64_t    Uint;
        char*       Str;
        double      Real;
    };
};

struct xoc_lexer {
    bool is_trusted;
    int row;
    int pos;
    int buf_pos;
    char* buf;
    token_t cur;
    token_t prev;
    pool_t* pool;
    map_t*  syms;
    info_t* info;
    log_t* log;
};

int  lexer_init (lexer_t* lex, const char* src, bool trusted, pool_t* pool, map_t* syms, info_t* info, log_t* log);
void lexer_free (lexer_t* lex);
void lexer_next (lexer_t* lex);
void lexer_nextf(lexer_t* lex);
bool lexer_check(lexer_t* lex, tokenkind_t kind);
void lexer_eat  (lexer_t* lex, tokenkind_t kind);
const char* lexer_mnemonic(tokenkind_t kind);
tokenkind_t lexer_trans_assign(tokenkind_t kind);

#endif /* XOC_LEXER_H */