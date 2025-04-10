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
            identname_t name;                           /** Identifier name */
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
    bool has_src;
    int row;
    int pos;
    int buf_pos;
    char* file;
    char* buf;
    token_t cur;
    token_t prev;
    mempool_t* pool;
    info_t* info;
    errp_t* errp;
};

int lexer_init(lexer_t* lex, mempool_t* pool, const char* file, const char* src, bool trusted, info_t* info, errp_t* errp);
void lexer_free(lexer_t* lex);
void lexer_next(lexer_t* lex);
void lexer_next_forced(lexer_t* lex);
bool lexer_check(lexer_t* lex, tokenkind_t kind);
const char* lexer_keyword(tokenkind_t kind);
void lexer_eat(lexer_t* lex, tokenkind_t kind);
tokenkind_t lexer_trans_assign(tokenkind_t kind);

#endif /* XOC_LEXER_H */