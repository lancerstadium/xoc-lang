#include <xoc_compiler.h>
#include <stdio.h>

void compiler_init(compiler_t* cp, const char* file, const char* src, compiler_option_t* opt) {

    // 1. Init all
    // -- Init components
    info_init   (&cp->info, file ? file : "main", src ? src : "main", 1, 1, 0);
    log_init    (&cp->log, &cp->info, log_fn_info);
    pool_init   (&cp->idts);
    pool_init   (&cp->blks);
    map_init    (&cp->sym_tbl);
    map_add     (&cp->sym_tbl, "main", 5);
    lexer_init  (&cp->lex, src, false, &cp->idts, &cp->sym_tbl, &cp->info, &cp->log);
    parser_init (&cp->prs, &cp->lex, &cp->blks, &cp->idts, &cp->sym_tbl);

    // -- Init compiler options
    cp->opt = *opt;

}



void compiler_free(compiler_t* cp) {
    // 1. Free all
    // -- Free components
    parser_free (&cp->prs);
    lexer_free  (&cp->lex);
    map_free    (&cp->sym_tbl);
    pool_free   (&cp->blks);
    pool_free   (&cp->idts);
    info_free   (&cp->info);
}



