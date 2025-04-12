/**
 * @file    xoc_parser.h
 * @brief   XOC Parser Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC Parser.
 * @note    Use standard bare C headers for cross-compatibility.
 */

#ifndef XOC_PARSER_H
#define XOC_PARSER_H

#include "xoc_lexer.h"

struct xoc_node {
    type_t* type;
    node_t* prevs;
    node_t* nexts;
};

struct xoc_parser {
    pool_t* blks;
    node_t* root;
    node_t* cur;
    int insn_pos;
    lexer_t* lex;

    info_t* info;
    log_t* log;
};


void parser_init(parser_t* parser, lexer_t* lex, pool_t* blks);
void parser_expr(parser_t* parser);
void parser_stmt(parser_t* parser);
void parser_free(parser_t* parser);


#endif /* XOC_PARSER_H */



