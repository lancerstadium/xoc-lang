#include <xoc_parser.h>
#include <xoc_types.h>

#include <stdio.h>
#include <stdlib.h>

static void parser_expr_leaf(parser_t* prs);
static void parser_expr_unop(parser_t* prs);

void parser_push_insts(parser_t* prs, inst_t* insts, int size) {
    char* res = pool_npush(prs->blks, (char**)&prs->blk_cur, (char*)insts, size);
    if(!res) {
        prs->log->fmt(prs->info, "Unable to push insts: %p(%u+%d/%u)", insts, pool_nsize(prs->blk_cur), size, pool_ncap(prs->blk_cur));
    }
}

// expr_leaf => int | real | ident
static void parser_expr_leaf(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_INT_LIT) {
        lexer_eat(lex, XOC_TOK_INT_LIT);
        prs->cur = (type_t){ .kind = XOC_TYPE_I64, .I64 = lex->cur.Int };
    } else if (lex->cur.kind == XOC_TOK_REAL_LIT) {
        lexer_eat(lex, XOC_TOK_REAL_LIT);
        prs->cur = (type_t){ .kind = XOC_TYPE_F32, .F32 = lex->cur.Real };
    } else if (lex->cur.kind == XOC_TOK_IDT) {
        lexer_eat(lex, XOC_TOK_IDT);
        prs->cur = (type_t){ .kind = XOC_TYPE_PTR, .WPtr = lex->cur.key };
    }
}

// expr_unop => '-' | '+' | '!' expr_leaf
static void parser_expr_unop(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_PLUS || 
        lex->cur.kind == XOC_TOK_MINUS || 
        lex->cur.kind == XOC_TOK_NOT) {
        lexer_next(lex);
        parser_expr_leaf(prs);
    }
    parser_push_insts(prs, &(inst_t){
        .op = XOC_OP_UNARY,
        .args = { [0] = prs->cur }
    }, 1);

}

// // expr_binop => expr_unop { '+' | '-' | '*' | '/' } expr_unop
// static void parser_expr_binop(parser_t* prs) {
//     lexer_t* lex = prs->lex;
//     parser_expr_unop(prs);
//     if (lex->cur.kind == XOC_TOK_PLUS || 
//         lex->cur.kind == XOC_TOK_MINUS || 
//         lex->cur.kind == XOC_TOK_MUL || 
//         lex->cur.kind == XOC_TOK_DIV) {
//         lexer_next(lex);
//         parser_expr_unop(prs);
//     }
// }


// expr => expr_leaf | expr_binop | expr_unop
void parser_expr(parser_t* prs) {
    prs->blk_cur = (inst_t*)pool_nalc(prs->blks, sizeof(inst_t), 1);
    prs->log->fmt(prs->info, "insts: %d/%d", pool_nsize(prs->blk_cur), pool_ncap(prs->blk_cur));
    parser_expr_unop(prs);
    prs->log->fmt(prs->info, "insts: %d/%d", pool_nsize(prs->blk_cur), pool_ncap(prs->blk_cur));
    printf("\ncur type: %d, res = %ld", prs->blk_cur[0].op, prs->blk_cur[0].args[0].I64);
    printf("\ncur type: %d, res = %ld", prs->blk_cur[1].op, prs->blk_cur[1].args[0].I64);
}

void parser_stmt(parser_t* prs) {
    // parse a statement
    lexer_t* lex = prs->lex;
    if (lex->cur.kind != XOC_TOK_EOL || lex->cur.kind != XOC_TOK_EOLI || lex->cur.kind != XOC_TOK_EOF) {
        parser_expr(prs);
    }
}

void parser_init(parser_t* prs, lexer_t* lex, pool_t* blks) {
    prs->blks = blks;
    prs->lex = lex;
    prs->info = lex->info;
    prs->log = lex->log;

    prs->cur.kind = XOC_TYPE_NONE;
    prs->cur.Ptr  = NULL;
}

void parser_free(parser_t* prs) {

}