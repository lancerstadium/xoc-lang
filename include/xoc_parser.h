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

struct xoc_operand {
    union {
        int8_t      Int8[8];
        uint8_t     UInt8[8];
        int16_t     Int16[4];
        uint16_t    UInt16[4];
        int32_t     Int32[2];
        uint32_t    UInt32[2];
        int64_t     Int64;
        uint64_t    UInt64;
        float       F32[2];
        double      F64;
        void*       Ptr;
        uint64_t    WPtr;
    };
};

struct xoc_inst {
    opcode_t    opc;
    operand_t   opr;
    type_t*     type;
};

struct xoc_insts {
    inst_t*     head;
    inst_t*     tail;
    int64_t     size;
};

struct xoc_funcctx {
    int64_t     offset;
    operand_t*  params;
    operand_t*  result;
};

struct xoc_parser {
    insts_t  code;
    lexer_t* lex;

    info_t* info;
    log_t* log;
};


void parser_init(parser_t* parser, lexer_t* lex);
void parser_run(parser_t* parser);
void parser_free(parser_t* parser);


#endif /* XOC_PARSER_H */



