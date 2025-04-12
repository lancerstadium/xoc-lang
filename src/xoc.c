#include <xoc_api.h>
#include <xoc_compiler.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    compiler_t cp;
    compiler_init(&cp, NULL, 
        "fn (xx: int) void\n"
        "xx_y = a * 19 + 1e-3"
        , 
        &(compiler_option_t){
            .argc = argc,
            .argv = argv,
            .is_filesys_enabled = true,
            .is_impllib_enabled = true,
        }
    );

    lexer_eat(&cp.lex, XOC_TOK_NONE);   // start
    lexer_eat(&cp.lex, XOC_TOK_FN);     // fn
    lexer_eat(&cp.lex, XOC_TOK_LPAR);   // (
    lexer_eat(&cp.lex, XOC_TOK_IDT);    // xx
    lexer_eat(&cp.lex, XOC_TOK_COLON);  // :
    lexer_eat(&cp.lex, XOC_TOK_IDT);    // int
    lexer_eat(&cp.lex, XOC_TOK_RPAR);   // )
    lexer_eat(&cp.lex, XOC_TOK_IDT);    // void
    lexer_eat(&cp.lex, XOC_TOK_IMPL_SEMICOL);    // \n
    lexer_eat(&cp.lex, XOC_TOK_IDT);
    lexer_eat(&cp.lex, XOC_TOK_EQ);
    lexer_eat(&cp.lex, XOC_TOK_IDT);
    lexer_eat(&cp.lex, XOC_TOK_MUL);
    lexer_eat(&cp.lex, XOC_TOK_INT);
    lexer_eat(&cp.lex, XOC_TOK_PLUS);
    lexer_eat(&cp.lex, XOC_TOK_REAL);

    // compiler_free(&cp);

    return 0;
}