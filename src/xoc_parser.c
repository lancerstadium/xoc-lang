#include <xoc_parser.h>
#include <xoc_types.h>

static void parser_expr_leaf(parser_t* parser);
static void parser_expr_unop(parser_t* parser);

// expr_leaf => int | real | ident
static void parser_expr_leaf(parser_t* parser) {
    lexer_t* lex = parser->lex;
    if (lex->cur.kind == XOC_TOK_INT_LIT) {

    }
}

// expr_unop => '-' | '+' | '!' expr_leaf
static void parser_expr_unop(parser_t* parser) {
    lexer_t* lex = parser->lex;
    if (lex->cur.kind == XOC_TOK_PLUS || 
        lex->cur.kind == XOC_TOK_MINUS || 
        lex->cur.kind == XOC_TOK_NOT) {
        lexer_next(lex);
        parser_expr_leaf(parser);
    }
}

// expr_binop => expr_unop { '+' | '-' | '*' | '/' } expr_unop
static void parser_expr_binop(parser_t* parser) {
    lexer_t* lex = parser->lex;
    parser_expr_unop(parser);
    if (lex->cur.kind == XOC_TOK_PLUS || 
        lex->cur.kind == XOC_TOK_MINUS || 
        lex->cur.kind == XOC_TOK_MUL || 
        lex->cur.kind == XOC_TOK_DIV) {
        lexer_next(lex);
        parser_expr_unop(parser);
    }
}


// expr => expr_leaf | expr_binop | expr_unop
void parser_expr(parser_t* parser) {
    

}

void parser_stmt(parser_t* parser) {
    // parse a statement
    lexer_t* lex = parser->lex;
    if (lex->cur.kind != XOC_TOK_EOL || lex->cur.kind != XOC_TOK_EOLI || lex->cur.kind != XOC_TOK_EOF) {
        parser_expr(parser);
    }
}

void parser_init(parser_t* ps, lexer_t* lex, pool_t* blks) {
    ps->blks = blks;
    ps->lex = lex;
    ps->info = lex->info;
    ps->log = lex->log;
}

void parser_free(parser_t* parser) {

}