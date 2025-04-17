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

struct xoc_parser {
    int iid;
    int bid;
    int tid;
    int lid;
    type_t cur;

    // int bid_cur;
    // int bid_src;
    // int iid_cur;
    // int iid_src;
    unsigned int symstk[256];
    int symstk_top;
    // unsigned int if_lbl;
    unsigned int sw_lbl;

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
