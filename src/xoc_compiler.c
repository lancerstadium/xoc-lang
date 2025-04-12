#include <xoc_compiler.h>
#include <stdio.h>

void compiler_init(compiler_t* cp, const char* file, const char* src, compiler_option_t* opt) {

    // 1. Init all
    // -- Init components
    info_init   (&cp->info, file ? file : "<unk>", src ? src : "<unk>", 1, 1, 0);
    log_init    (&cp->log, &cp->info, log_fn_info);
    pool_init   (&cp->pool);
    lexer_init  (&cp->lex, src, false, &cp->pool, &cp->info, &cp->log);
    parser_init (&cp->ps, &cp->lex);

    // -- Init compiler options
    cp->opt = *opt;

}



void compiler_free(compiler_t* cp) {
    // 1. Free all
    // -- Free components
    parser_free (&cp->ps);
    lexer_free  (&cp->lex);
    pool_free   (&cp->pool);
}



