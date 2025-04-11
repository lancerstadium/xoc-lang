#include <xoc_api.h>
#include <xoc_compiler.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    compiler_t cp;
    compiler_init(&cp, NULL, "xx = 1e3 + 1", &(compiler_option_t){
        .argc = argc,
        .argv = argv,
        .is_filesys_enabled = true,
        .is_impllib_enabled = true,
    });

    lexer_eat(&cp.lex, XOC_TOK_NONE);
    lexer_eat(&cp.lex, XOC_TOK_REAL);

    compiler_free(&cp);

    return 0;
}