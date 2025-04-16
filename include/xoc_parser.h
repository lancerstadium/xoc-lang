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
#include "xoc_types.h"

struct xoc_inst {
    unsigned int label;
    opcode_t     op;
    type_t       args[4];
};

typedef struct xoc_node node_t;

struct xoc_node {
    inst_t* blk;

    int num_pred;
    int num_succ;
    node_t* pred;
    node_t* succ;

    node_t* next;
};

struct xoc_parser {
    int iid;
    int bid;
    int tid;
    type_t cur;

    node_t* root;

    int bid_cur;
    int bid_src;
    unsigned int label;

    inst_t* blk_cur;
    pool_t* blks;
    map_t*  syms;
    lexer_t* lex;

    info_t* info;
    log_t* log;
};


void parser_init(parser_t* prs, lexer_t* lex, pool_t* blks, map_t* syms);
void parser_stmt(parser_t* prs);
void parser_free(parser_t* prs);


#endif /* XOC_PARSER_H */
