#include <xoc_api.h>
#include <xoc_compiler.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    compiler_t cp;
    compiler_init(&cp, NULL, 
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

    lexer_eat(&cp.lex, XOC_TOK_IDT);
    lexer_eat(&cp.lex, XOC_TOK_EQ);
    lexer_eat(&cp.lex, XOC_TOK_IDT);
    lexer_eat(&cp.lex, XOC_TOK_MUL);
    lexer_eat(&cp.lex, XOC_TOK_INT_LIT);
    lexer_eat(&cp.lex, XOC_TOK_PLUS);
    lexer_eat(&cp.lex, XOC_TOK_REAL_LIT);

    // compiler_free(&cp);

    return 0;
}