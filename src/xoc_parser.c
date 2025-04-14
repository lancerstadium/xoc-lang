#include <xoc_parser.h>
#include <xoc_types.h>

#include <stdio.h>
#include <stdlib.h>

static void parser_expr_leaf(parser_t* prs);
static void parser_expr_unop(parser_t* prs);
static void parser_expr_binop(parser_t* prs);

type_t type_sym(tokenkind_t tk) {
    return (type_t){ .kind = XOC_TYPE_I64, .I64 = tk };
}

type_t type_tmp(int id) {
    return (type_t){ .kind = XOC_TYPE_WPTR, .WPtr = id };
}

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

// expr_unop => {'-' | '+' | '!'} expr_leaf
static void parser_expr_unop(parser_t* prs) {
    lexer_t* lex = prs->lex;
    type_t sym;
    if (lex->cur.kind == XOC_TOK_PLUS || 
        lex->cur.kind == XOC_TOK_MINUS || 
        lex->cur.kind == XOC_TOK_NOT) {
        printf("\nsymbol: %s", lexer_mnemonic(lex->prev.kind));
        sym = type_sym(lex->prev.kind);
        lexer_next(lex);
        printf("\nsymbol: %s", lexer_mnemonic(lex->prev.kind));
    } else {
        sym = type_sym(XOC_TOK_NONE);
    }
    parser_expr_leaf(prs);
    parser_push_insts(prs, &(inst_t){
        .op     = XOC_OP_UNARY,
        .args   = { [0] = sym, [1] = type_tmp(prs->tid), [2] = prs->cur }
    }, 1);
    prs->cur = type_tmp(prs->tid);
    prs->tid++;
}

// expr_binop => expr_unop '+' | '-' | '*' | '/' expr_unop
static void parser_expr_binop(parser_t* prs) {
    lexer_t* lex = prs->lex;
    type_t lhs, rhs, sym;
    parser_expr_unop(prs);
    lhs = prs->cur;
    if (lex->cur.kind == XOC_TOK_PLUS || 
        lex->cur.kind == XOC_TOK_MINUS || 
        lex->cur.kind == XOC_TOK_MUL || 
        lex->cur.kind == XOC_TOK_DIV) {
        printf("\nsymbol: %s", lexer_mnemonic(lex->prev.kind));
        sym = type_sym(lex->prev.kind);
        lexer_next(lex);
        printf("\nsymbol: %s", lexer_mnemonic(lex->prev.kind));
    } else {
        return;
    }
    parser_expr_unop(prs);
    rhs = prs->cur;
    parser_push_insts(prs, &(inst_t){
        .op     = XOC_OP_BINARY,
        .args   = { [0] = sym, [1] = type_tmp(prs->tid), [2] = lhs, [3] = rhs }
    }, 1);
    prs->cur = type_tmp(prs->tid);
    prs->tid++;
}


// expr => expr_leaf | expr_binop | expr_unop
void parser_expr(parser_t* prs) {
    prs->blk_cur = (inst_t*)pool_nalc(prs->blks, sizeof(inst_t), 1);
    prs->log->fmt(prs->info, "insts: %d/%d", pool_nsize(prs->blk_cur), pool_ncap(prs->blk_cur));
    parser_expr_binop(prs);
    prs->log->fmt(prs->info, "insts: %d/%d", pool_nsize(prs->blk_cur), pool_ncap(prs->blk_cur));
    printf("\ncur type: %d %s, res = [%ld,%ld,%ld]", prs->blk_cur[0].op, lexer_mnemonic(prs->blk_cur[0].args[0].I64), prs->blk_cur[0].args[1].WPtr, prs->blk_cur[0].args[2].I64, prs->blk_cur[0].args[3].I64);
    printf("\ncur type: %d %s, res = [%ld,%ld,%ld]", prs->blk_cur[1].op, lexer_mnemonic(prs->blk_cur[1].args[0].I64), prs->blk_cur[1].args[1].WPtr, prs->blk_cur[1].args[2].I64, prs->blk_cur[1].args[3].I64);
    printf("\ncur type: %d %s, res = [%ld,%ld,%ld]", prs->blk_cur[2].op, lexer_mnemonic(prs->blk_cur[2].args[0].I64), prs->blk_cur[2].args[1].WPtr, prs->blk_cur[2].args[2].I64, prs->blk_cur[2].args[3].I64);
}

void parser_stmt(parser_t* prs) {
    // parse a statement
    lexer_t* lex = prs->lex;
    if (lex->cur.kind != XOC_TOK_EOL || lex->cur.kind != XOC_TOK_EOLI || lex->cur.kind != XOC_TOK_EOF) {
        parser_expr(prs);
    }
}

void parser_init(parser_t* prs, lexer_t* lex, pool_t* blks) {
    prs->tid = 0;
    prs->blks = blks;
    prs->lex = lex;
    prs->info = lex->info;
    prs->log = lex->log;

    prs->cur.kind = XOC_TYPE_NONE;
    prs->cur.Ptr  = NULL;
}

void parser_free(parser_t* prs) {

}