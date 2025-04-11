#include <xoc_parser.h>
#include <stdio.h>

void parser_init(parser_t* ps, lexer_t* lex) {
    ps->code.head = NULL;
    ps->code.tail = NULL;
    ps->code.size = 0;
    ps->lex = lex;
    ps->info = lex->info;
    ps->log = lex->log;
}


void parser_run(parser_t* parser) {
    
}

void parser_free(parser_t* parser) {

}