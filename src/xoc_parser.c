#include <xoc_parser.h>
#include <xoc_types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int  parser_term_level(parser_t* prs, tokenkind_t tk);
static void parser_param_list(parser_t* prs);
static void parser_selectors(parser_t* prs);
static void parser_qualident(parser_t* prs);
static void parser_primary(parser_t* prs);
static void parser_ptrtype(parser_t* prs);
static void parser_strtype(parser_t* prs);
static void parser_enumitem(parser_t* prs);
static void parser_enumtype(parser_t* prs);
static void parser_vectype(parser_t* prs);
static void parser_maptype(parser_t* prs);
static void parser_identlist(parser_t* prs);
static void parser_typedidentlist(parser_t* prs);
static void parser_signature(parser_t* prs);
static void parser_structtype(parser_t* prs);
static void parser_interfacetype(parser_t* prs);
static void parser_closuretype(parser_t* prs);
static void parser_type(parser_t* prs);
static void parser_typecast(parser_t* prs); 
static void parser_veclit(parser_t* prs); 
static void parser_maplit(parser_t* prs);
static void parser_structlit(parser_t* prs); 
static void parser_closurelit(parser_t* prs);
static void parser_compositelit(parser_t* prs);
static void parser_enumconst(parser_t* prs);
static void parser_designator(parser_t* prs);
static void parser_designatorlist(parser_t* prs);
static void parser_decl_assignlist(parser_t* prs);
static void parser_decl_shortval(parser_t* prs);
static void parser_factor(parser_t* prs);
static void parser_term_n(parser_t* prs, int n);
static void parser_expr(parser_t* prs);
static void parser_exprlist(parser_t* prs);
static void parser_stmt_assign(parser_t* prs);
static void parser_stmt_assignlist(parser_t* prs);
static void parser_stmt_simple(parser_t* prs);
static void parser_stmt_if(parser_t* prs);
static void parser_expr_case(parser_t* prs);
static void parser_stmt_switch(parser_t* prs);
static void parser_forheader(parser_t* prs);
static void parser_forinheader(parser_t* prs);
static void parser_stmt_for(parser_t* prs);
static void parser_stmt_return(parser_t* prs);
static void parser_stmtlist(parser_t* prs);
static void parser_block(parser_t* prs);


static const char* type_mnemonic_tbl[] = {
    [XOC_TYPE_NONE]     = "none",
    [XOC_TYPE_TMP]      = "$",
    [XOC_TYPE_IDT]      = "$",
    [XOC_TYPE_BLK]      = "#",
    [XOC_TYPE_LBL]      = "%",
    [XOC_TYPE_I8]       = "i8",
    [XOC_TYPE_I16]      = "i16",
    [XOC_TYPE_I32]      = "i32",
    [XOC_TYPE_I64]      = "i64",
    [XOC_TYPE_U8]       = "u8",
    [XOC_TYPE_U16]      = "u16",
    [XOC_TYPE_U32]      = "u32",
    [XOC_TYPE_U64]      = "u64",
    [XOC_TYPE_F32]      = "f32",
    [XOC_TYPE_F64]      = "f64",
    [XOC_TYPE_BOOL]     = "bool",
    [XOC_TYPE_VOID]     = "void",
    [XOC_TYPE_NULL]     = "null",
    [XOC_TYPE_PTR]      = "ptr",
    [XOC_TYPE_WPTR]     = "wptr",
    [XOC_TYPE_CHAR]     = "char",
    [XOC_TYPE_STR]      = "str",
};

void type_info(type_t* type, char* buf, int len, map_t* syms) {
    switch (type->kind) {
        case XOC_TYPE_SYM:  snprintf(buf, len, "%s", lexer_mnemonic(type->WPtr)); break;
        case XOC_TYPE_TMP:  snprintf(buf, len, "%s%ld", type_mnemonic_tbl[type->kind], type->WPtr); break;
        case XOC_TYPE_IDT:  snprintf(buf, len, "%s%s", type_mnemonic_tbl[type->kind], map_get(syms, type->WPtr)); break;
        case XOC_TYPE_BLK:  snprintf(buf, len, "%s%ld", type_mnemonic_tbl[type->kind], type->WPtr); break;
        case XOC_TYPE_LBL:  snprintf(buf, len, "%s%s", type_mnemonic_tbl[type->kind], map_get(syms, type->WPtr)); break;
        case XOC_TYPE_I64:  snprintf(buf, len, "%ld:%s", type->I64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_F32:  snprintf(buf, len, "%f:%s", type->F32, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_F64:  snprintf(buf, len, "%lf:%s", type->F64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_CHAR: snprintf(buf, len, "'%c':%s", (char)type->I64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_STR:  snprintf(buf, len, "\"%s\":%s", (char*)type->WPtr, type_mnemonic_tbl[type->kind]); break;
        default:            snprintf(buf, len, "%s", type_mnemonic_tbl[type->kind]); break;
    }
}

void inst_info(inst_t* inst, char* buf, int len, map_t* syms) {
    char label[64];
    char arg[4][64];
    if(inst->label != 0) {
        snprintf(label, 64, " %%%-46s │\n│", map_get(syms, inst->label));
    } else {
        label[0] = 0;
    }
    type_info(&inst->args[0], arg[0], 64, syms);
    type_info(&inst->args[1], arg[1], 64, syms);
    type_info(&inst->args[2], arg[2], 64, syms);
    type_info(&inst->args[3], arg[3], 64, syms);
    switch (inst->op) {
        case XOC_OP_PUSH:       snprintf(buf, len, "%s  PUSH       %s = %s"         , label, arg[0], arg[1]); break;
        case XOC_OP_UNARY:      snprintf(buf, len, "%s  UNARY      %s = %s %s"      , label, arg[1], arg[0], arg[2]); break;
        case XOC_OP_BINARY:     snprintf(buf, len, "%s  BINARY     %s = %s %s %s"   , label, arg[1], arg[2], arg[0], arg[3]); break;
        case XOC_OP_ASSIGN:     snprintf(buf, len, "%s  ASSIGN     %s = %s"         , label, arg[0], arg[1]); break;
        case XOC_OP_GOTO:       snprintf(buf, len, "%s  GOTO       %s"              , label, arg[0]); break;
        case XOC_OP_GOTO_IF:    snprintf(buf, len, "%s  GOTO_IF    %s, %s"          , label, arg[0], arg[1]); break;
        case XOC_OP_GOTO_IFN:   snprintf(buf, len, "%s  GOTO_IFN   %s, %s"          , label, arg[0], arg[1]); break;
        case XOC_OP_GOTO_IFEQ:  snprintf(buf, len, "%s  GOTO_IFEQ  %s, %s == %s"    , label, arg[0], arg[1], arg[2]); break;
        case XOC_OP_GOTO_IFNE:  snprintf(buf, len, "%s  GOTO_IFNE  %s, %s != %s"    , label, arg[0], arg[1], arg[2]); break;
        default:                snprintf(buf, len, "%s  UNKNOW     %d"              , label, inst->op); break;
    }
}

void blk_info(inst_t* blk, char* buf, int size, map_t* syms) {
    int i;
    if(pool_nsize(blk) == 0) {
        if(blk[0].label) {
            printf("\n│ %%%-46s │", map_get(syms, blk[0].label));
        }
    }
    for (i = 0; i < pool_nsize(blk); i++) {
        inst_info(&blk[i], buf, 300, syms);
        if(blk[i].label) {
            printf("\n│%-105s│", buf);
        } else {
            printf("\n│%-49s│", buf);
        }
    }
}

type_t type_sym(tokenkind_t tk) {
    return (type_t){ .kind = XOC_TYPE_SYM, .WPtr = tk };
}

type_t type_tmp(int id) {
    return (type_t){ .kind = XOC_TYPE_TMP, .WPtr = id };
}

type_t type_blk(int id) {
    return (type_t){ .kind = XOC_TYPE_BLK, .WPtr = id };
}

type_t type_lbl(unsigned int key) {
    return (type_t){ .kind = XOC_TYPE_LBL, .WPtr = key };
}

int parser_symstk_push(parser_t* prs, unsigned int key) {
    if (prs->symstk_top + 1 >= 256) {
        prs->log->fmt(prs->info, "Symbol stark overflow: %u", prs->symstk_top + 1);
        return -1;
    }
    prs->symstk[++prs->symstk_top] = key;
    return prs->symstk_top;
}

unsigned int parser_symstk_pop(parser_t* prs) {
    if (prs->symstk_top <= -1) {
        prs->log->fmt(prs->info, "Symbol stark underflow: %u", prs->symstk_top);
        return 0;
    }
    return prs->symstk[prs->symstk_top--];
}

unsigned int parser_symstk_top(parser_t* prs) {
    return prs->symstk_top < 0 ? 0 : prs->symstk[prs->symstk_top];
}

void parser_symstk_set(parser_t* prs, unsigned int key) {
    if (prs->symstk_top <= -1) {
        prs->log->fmt(prs->info, "Symbol stark underflow: %u", prs->symstk_top);
        return;
    } else {
        prs->symstk[prs->symstk_top] = key;
    }
}


inst_t* parser_blk_alc(parser_t* prs, int cap) {
    inst_t* blk_cur = (inst_t*)pool_nalc(prs->blks, sizeof(inst_t), cap);
    prs->bid++;
    prs->blk_cur = blk_cur;
    prs->iid = 0;
    return prs->blk_cur;
}

static int parser_term_level(parser_t* prs, tokenkind_t tk) {
    switch (tk) {
        case XOC_TOK_MUL: return 0;
        case XOC_TOK_DIV: return 0;
        case XOC_TOK_MOD: return 0;
        case XOC_TOK_SHL: return 0;
        case XOC_TOK_SHR: return 0;
        case XOC_TOK_AND: return 0;

        case XOC_TOK_PLUS: return 1;
        case XOC_TOK_MINUS: return 1;
        case XOC_TOK_OR: return 1;
        case XOC_TOK_XOR: return 1;

        case XOC_TOK_EQEQ: return 2;
        case XOC_TOK_NOTEQ: return 2;
        case XOC_TOK_LESS: return 2;
        case XOC_TOK_LESSEQ: return 2;
        case XOC_TOK_GREATER: return 2;
        case XOC_TOK_GREATEREQ: return 2;

        case XOC_TOK_ANDAND: return 3;
        case XOC_TOK_OROR: return 4;
        default: return -1;
    }
}

void parser_push_insts(parser_t* prs, inst_t* insts, int size) {
    unsigned int org_lbl = prs->blk_cur[0].label;
    char* res = pool_npush(prs->blks, (char**)&prs->blk_cur, (char*)insts, size);
    if(!res) {
        prs->log->fmt(prs->info, "Unable to push insts: %p(%u+%d/%u)", insts, pool_nsize(prs->blk_cur), size, pool_ncap(prs->blk_cur));
    }
    if(org_lbl) {
        prs->blk_cur[0].label = org_lbl;
    }
    prs->iid++;
}

inst_t* parser_blk_swc(parser_t* prs, int bid) {
    inst_t* cur = (inst_t*)pool_nat(prs->blks, bid);
    if(cur) {
        prs->blk_cur = cur;
        prs->iid = pool_nsize(cur);
        return cur;
    } else {
        prs->log->fmt(prs->info, "Unable to switch blk[%u]: %p", bid, cur);
        return NULL;
    }
}

// param_list => '(' { expr {',' expr } } ')'
static void parser_param_list(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_LPAR) {
        lexer_next(lex);
        if (lex->cur.kind != XOC_TOK_RPAR) {
            parser_expr(prs);
            while (lex->cur.kind == XOC_TOK_COMMA) {
                lexer_next(lex);
                parser_expr(prs);
            }
        }
        lexer_eat(lex, XOC_TOK_RPAR);
    }
}

// selectors => {'^' | '[' expr ']' | '.' ident | param_list }
static void parser_selectors(parser_t* prs) {
    lexer_t* lex = prs->lex;
    while (lex->cur.kind == XOC_TOK_CARET || lex->cur.kind == XOC_TOK_LBRACKET || lex->cur.kind == XOC_TOK_PERIOD || lex->cur.kind == XOC_TOK_LPAR) {
        if (lex->cur.kind == XOC_TOK_CARET) {
            lexer_next(lex);
        } else if (lex->cur.kind == XOC_TOK_LBRACKET) {
            lexer_next(lex);
            parser_expr(prs);
            lexer_eat(lex, XOC_TOK_RBRACKET);
        } else if (lex->cur.kind == XOC_TOK_PERIOD) {
            lexer_next(lex);
            if (lex->cur.kind == XOC_TOK_IDT) {
                lexer_next(lex);
            } else {
                prs->log->fmt(prs->info, "Expect identifier after `.` in selectors");
            }
        } else if (lex->cur.kind == XOC_TOK_LPAR) {
            parser_param_list(prs);
        }
    }
}

// qualident => ident ['::' ident]
static void parser_qualident(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        // type_t fd = (type_t){ .kind = XOC_TYPE_IDT, .WPtr = xoc_hash("main") };
        prs->cur = (type_t){ .kind = XOC_TYPE_IDT, .WPtr = lex->cur.key };
        lexer_next(lex);
        if (lex->cur.kind == XOC_TOK_COLONCOLON) {
            lexer_next(lex);
            if (lex->cur.kind == XOC_TOK_IDT) {
                prs->cur = (type_t){ .kind = XOC_TYPE_IDT, .WPtr = lex->cur.key };
            }
        }
    }
}

// primary => qualident [param_list]
static void parser_primary(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_qualident(prs);
        if (lex->cur.kind == XOC_TOK_LPAR) {
            parser_param_list(prs);
        }
    }
}

// ptrtype => ['weak'] '^' type
static void parser_ptrtype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_CARET) {
        lexer_next(lex);
        parser_type(prs);
    } else if (lex->cur.kind == XOC_TOK_WEAK) {
        lexer_next(lex);
        lexer_eat(lex, XOC_TOK_CARET);
        parser_type(prs);
    }
}

// strtype => 'str'
static void parser_strtype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_STR) {
        lexer_next(lex);
    }
}

// enumitem => ident ['=' expr]
static void parser_enumitem(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        lexer_next(lex);
        if (lex->cur.kind == XOC_TOK_EQ) {
            lexer_next(lex);
            parser_expr(prs);
        }
    }
}

// enumtype => 'enum' [':' type] '{' { enumitem ';' } '}'
static void parser_enumtype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_ENUM) {
        lexer_next(lex);
        if (lex->cur.kind == XOC_TOK_COLON) {
            lexer_next(lex);
            parser_type(prs);
        }
        lexer_eat(lex, XOC_TOK_LBRACE);
        while (lex->cur.kind != XOC_TOK_RBRACE) {
            parser_enumitem(prs);
            lexer_eat(lex, XOC_TOK_SEMICOLON);
        }
        lexer_eat(lex, XOC_TOK_RBRACE);
    }
}

// vectype => '[' [expr] ']' type
static void parser_vectype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_LBRACKET) {
        lexer_next(lex);
        if (lex->cur.kind != XOC_TOK_RBRACKET) {
            parser_expr(prs);
        }
        lexer_eat(lex, XOC_TOK_RBRACKET);
        parser_type(prs);
    }
}

// maptype => 'map' '[' type ']' type
static void parser_maptype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_MAP) {
        lexer_next(lex);
        lexer_eat(lex, XOC_TOK_LBRACKET);
        parser_type(prs);
        lexer_eat(lex, XOC_TOK_RBRACKET);
        parser_type(prs);
    }
}

// identlist => ident ['*'] { ',' ident ['*'] }
static void parser_identlist(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        lexer_next(lex);
        while (lex->cur.kind == XOC_TOK_MUL) {
            lexer_next(lex);
        }
        while (lex->cur.kind == XOC_TOK_COMMA) {
            lexer_next(lex);
            if (lex->cur.kind == XOC_TOK_IDT) {
                lexer_next(lex);
                if (lex->cur.kind == XOC_TOK_MUL) {
                    lexer_next(lex);
                }
            }
        }
    }
}

// typedidentlist => identlist ':' ['..'] type
static void parser_typedidentlist(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_identlist(prs);
        lexer_eat(lex, XOC_TOK_COLON);
        if (lex->cur.kind == XOC_TOK_ELLIPSIS) {
            lexer_next(lex);
        }
        parser_type(prs);
    }
}

// signature => '(' [typedidentlist ['=' expr] {',' typedidentlist ['=' expr] }] ')' [':' (type | '(' type {',' type } ')')]
static void parser_signature(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_LPAR) {
        lexer_next(lex);
        if (lex->cur.kind != XOC_TOK_RPAR) {
            parser_typedidentlist(prs);
            if (lex->cur.kind == XOC_TOK_EQ) {
                lexer_next(lex);
                parser_expr(prs);
            }
            while (lex->cur.kind == XOC_TOK_COMMA) {
                lexer_next(lex);
                parser_typedidentlist(prs);
                if (lex->cur.kind == XOC_TOK_EQ) {
                    lexer_next(lex);
                    parser_expr(prs);
                }
            }
        }
        lexer_eat(lex, XOC_TOK_RPAR);
        if (lex->cur.kind == XOC_TOK_COLON) {
            lexer_next(lex);
            if (lex->cur.kind == XOC_TOK_LPAR) {
                lexer_next(lex);
                parser_type(prs);
                while (lex->cur.kind == XOC_TOK_COMMA) {
                    lexer_next(lex);
                    parser_type(prs);
                }
                lexer_eat(lex, XOC_TOK_RPAR);
            } else {
                parser_type(prs);
            }
        }
    }
}

// structtype => 'struct' '{' { typedidentlist ';' } '}'
static void parser_structtype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_STRUCT) {
        lexer_next(lex);
        lexer_eat(lex, XOC_TOK_LBRACE);
        while (lex->cur.kind != XOC_TOK_RBRACE) {
            parser_typedidentlist(prs);
            lexer_eat(lex, XOC_TOK_SEMICOLON);
        }
        lexer_eat(lex, XOC_TOK_RBRACE);
    }
}

// interfacetype => 'interface' '{' { (ident signature | qualident) ';' } '}'
static void parser_interfacetype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_INTERFACE) {
        lexer_next(lex);
        lexer_eat(lex, XOC_TOK_LBRACE);
        while (lex->cur.kind != XOC_TOK_RBRACE) {
            if (lex->cur.kind == XOC_TOK_IDT) {
                lexer_next(lex);
                parser_signature(prs);
            } else {
                parser_qualident(prs);
            }
            lexer_eat(lex, XOC_TOK_SEMICOLON);
        }
    }
}

// closuretype => 'fn' signature
static void parser_closuretype(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_FN) {
        lexer_next(lex);
        parser_signature(prs);
    }
}

static bool parser_istype(parser_t* prs) {
    tokenkind_t tk = prs->lex->cur.kind;
    return tk == XOC_TOK_IDT ||
           tk == XOC_TOK_WEAK ||
           tk == XOC_TOK_CARET ||
           tk == XOC_TOK_STR ||
           tk == XOC_TOK_ENUM ||
           tk == XOC_TOK_LBRACKET ||
           tk == XOC_TOK_MAP ||
           tk == XOC_TOK_STRUCT ||
           tk == XOC_TOK_INTERFACE ||
           tk == XOC_TOK_FN;
}

// type => qualident | ptrtype | strtype | enumtype | vectype | maptype | structtype | interfacetype | closuretype
static void parser_type(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_qualident(prs);
    } else if (lex->cur.kind == XOC_TOK_WEAK || lex->cur.kind == XOC_TOK_CARET) {
        parser_ptrtype(prs);
    } else if (lex->cur.kind == XOC_TOK_STR) {
        parser_strtype(prs);
    } else if (lex->cur.kind == XOC_TOK_ENUM) {
        parser_enumtype(prs);
    } else if (lex->cur.kind == XOC_TOK_LBRACKET) {
        parser_vectype(prs);
    } else if (lex->cur.kind == XOC_TOK_MAP) {
        parser_maptype(prs);
    } else if (lex->cur.kind == XOC_TOK_STRUCT) {
        parser_structtype(prs);
    } else if (lex->cur.kind == XOC_TOK_INTERFACE) {
        parser_interfacetype(prs);
    } else if (lex->cur.kind == XOC_TOK_FN) {
        parser_closuretype(prs);
    }
}

// typecast => type '(' expr ')'
static void parser_typecast(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (parser_istype(prs)) {
        parser_type(prs);
        lexer_eat(lex, XOC_TOK_LPAR);
        parser_expr(prs);
        lexer_eat(lex, XOC_TOK_RPAR);
    }
}

// veclit => '{' [expr {',' expr} [',']] '}'
static void parser_veclit(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_LBRACE) {
        lexer_next(lex);
        if (lex->cur.kind != XOC_TOK_RBRACE) {
            parser_expr(prs);
            while (lex->cur.kind == XOC_TOK_COMMA) {
                lexer_next(lex);
                if (lex->cur.kind == XOC_TOK_RBRACE) {
                    break;
                }
                parser_expr(prs);
            }
        }
        lexer_eat(lex, XOC_TOK_RBRACE);
    }
}

// maplit => '{' [expr ':' expr {',' expr ':' expr} [',']] '}'
static void parser_maplit(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_LBRACE) {
        lexer_next(lex);
        if (lex->cur.kind != XOC_TOK_RBRACE) {
            parser_expr(prs);
            lexer_eat(lex, XOC_TOK_COLON);
            parser_expr(prs);
            while (lex->cur.kind == XOC_TOK_COMMA) {
                lexer_next(lex);
                if (lex->cur.kind == XOC_TOK_RBRACE) {
                    break;
                }
                parser_expr(prs);
                lexer_eat(lex, XOC_TOK_COLON);
                parser_expr(prs);
            }
        }
        lexer_eat(lex, XOC_TOK_RBRACE);
    }
}

// structlit => '{' [[ident ':'] expr {',' [ident ':'] expr} [',']] '}'
static void parser_structlit(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_LBRACE) {
        lexer_next(lex);
        if (lex->cur.kind != XOC_TOK_RBRACE) {
            if (lex->cur.kind == XOC_TOK_IDT) {
                lexer_next(lex);
                lexer_eat(lex, XOC_TOK_COLON);
            }
            parser_expr(prs);
            while (lex->cur.kind == XOC_TOK_COMMA) {
                lexer_next(lex);
                if (lex->cur.kind == XOC_TOK_RBRACE) {
                    break;
                } else if (lex->cur.kind == XOC_TOK_IDT) {
                    lexer_next(lex);
                    lexer_eat(lex, XOC_TOK_COLON);
                }
                parser_expr(prs);
            }
        }
        lexer_eat(lex, XOC_TOK_RBRACE);
    }
}

// closurelit => ['|' ident {',' ident} '|'] block
static void parser_closurelit(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_OR) {
        lexer_next(lex);
        while (lex->cur.kind == XOC_TOK_IDT) {
            lexer_next(lex);
            if (lex->cur.kind == XOC_TOK_COMMA) {
                lexer_next(lex);
            }
        }
        lexer_eat(lex, XOC_TOK_OR);
    }
    parser_block(prs);
}

// compositelit => [type] (veclit | maplit | structlit | closurelit)
static void parser_compositelit(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_type(prs);
    if (lex->cur.kind == XOC_TOK_LBRACE) {
        parser_veclit(prs);
    } else if (lex->cur.kind == XOC_TOK_LPAR) {
        parser_maplit(prs);
    } else if (lex->cur.kind == XOC_TOK_LBRACE) {
        parser_structlit(prs);
    } else if (lex->cur.kind == XOC_TOK_OR) {
        parser_closurelit(prs);
    }
}

// enumconst => [type] '.' ident
static void parser_enumconst(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_type(prs);
    if (lex->cur.kind == XOC_TOK_PERIOD) {
        lexer_eat(lex, XOC_TOK_PERIOD);
        lexer_eat(lex, XOC_TOK_IDT);
    }
}

// designator => ( primary | typecast | compositelit | enumconst ) selectors
static void parser_designator(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_primary(prs);
    parser_selectors(prs);
}

// designatorlist => designator { ',' designator }
static void parser_designatorlist(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_designator(prs);
    while (lex->cur.kind == XOC_TOK_COMMA) {
        lexer_next(lex);
        parser_designator(prs);
    }
}

// decl_assignlist => identlist ':=' exprlist
static void parser_decl_assignlist(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_identlist(prs);
        lexer_eat(lex, XOC_TOK_COLONEQ);
        parser_exprlist(prs);
    }
}

// decl_shortval => decl_assignlist
static void parser_decl_shortval(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_decl_assignlist(prs);
    }
}

// factor => int | real | char | str | ident | ('-' | '+' | '!' | '~' | '&') factor | '(' expr ')'
static void parser_factor(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_INT_LIT ||
        lex->cur.kind == XOC_TOK_REAL_LIT ||
        lex->cur.kind == XOC_TOK_CHAR_LIT ||
        lex->cur.kind == XOC_TOK_STR_LIT ||
        lex->cur.kind == XOC_TOK_IDT) {
        tokenkind_t tk = lex->cur.kind;
        lexer_eat(lex, tk);
        switch (tk) {
            case XOC_TOK_INT_LIT: prs->cur = (type_t){ .kind = XOC_TYPE_I64 , .I64 = lex->cur.Int }; break;
            case XOC_TOK_REAL_LIT: prs->cur = (type_t){ .kind = XOC_TYPE_F64 , .F64 = lex->cur.Real }; break;
            case XOC_TOK_CHAR_LIT: prs->cur = (type_t){ .kind = XOC_TYPE_CHAR, .I64 = lex->cur.Int }; break;
            case XOC_TOK_STR_LIT: prs->cur = (type_t){ .kind = XOC_TYPE_STR, .WPtr = lex->cur.key }; break;
            case XOC_TOK_IDT: prs->cur = (type_t){ .kind = XOC_TYPE_IDT, .WPtr = lex->cur.key }; break;
            default: prs->cur = (type_t){ .kind = XOC_TYPE_NONE , .Ptr = NULL }; break;
        }
        // parser_push_insts(prs, &(inst_t){
        //     .op     = XOC_OP_PUSH,
        //     .args   = { [0] = type_tmp(prs->tid), [1] = prs->cur }
        // }, 1);
        // prs->cur = type_tmp(prs->tid);
        // prs->tid++;
    } else if (lex->cur.kind == XOC_TOK_PLUS || 
        lex->cur.kind == XOC_TOK_MINUS || 
        lex->cur.kind == XOC_TOK_NOT ||
        lex->cur.kind == XOC_TOK_XOR ||
        lex->cur.kind == XOC_TOK_AND) {
        type_t sym;
        sym = type_sym(lex->prev.kind);
        lexer_next(lex);
        parser_factor(prs);
        parser_push_insts(prs, &(inst_t){
            .op     = XOC_OP_UNARY,
            .args   = { [0] = sym, [1] = type_tmp(prs->tid), [2] = prs->cur }
        }, 1);
        prs->cur = type_tmp(prs->tid);
        prs->tid++;
    } else if (lex->cur.kind == XOC_TOK_LPAR) {
        lexer_eat(lex, XOC_TOK_LPAR);
        parser_expr(prs);
        lexer_eat(lex, XOC_TOK_RPAR);
    }
}

// term[n-1] => term[n-1] {(
//       0 '*' | '/' | '%' | '<<' | '>>' | '&' |
//       1 '+' | '-' | '|' | '~' |
//       2 '==' | '!=' | '<' | '<=' | '>' | '>='
//       3 '&&' |
//       4 '||') term[n-1]}
static void parser_term_n(parser_t* prs, int n) {
    lexer_t* lex = prs->lex;
    type_t lhs, rhs, sym;
    int level = n;
    if (n == 0) {
        parser_factor(prs);
        lhs = prs->cur;
        level = parser_term_level(prs, lex->cur.kind);
        if(level == 0) {
            sym = type_sym(lex->prev.kind);
            lexer_next(lex);
            parser_factor(prs);
            rhs = prs->cur;
        } else {
            return;
        }
    } else if(n > 0 && n < 5) {
        parser_term_n(prs, n-1);
        lhs = prs->cur;
        level = parser_term_level(prs, lex->cur.kind);
        if(level == n-1) {
            sym = type_sym(lex->prev.kind);
            lexer_next(lex);
            parser_term_n(prs, n-1);
            rhs = prs->cur;
        } else {
            return;
        }
    } else {
        return;
    }
    parser_push_insts(prs, &(inst_t){
        .op     = XOC_OP_BINARY,
        .args   = { [0] = sym, [1] = type_tmp(prs->tid), [2] = lhs, [3] = rhs }
    }, 1);
    prs->cur = type_tmp(prs->tid);
    prs->tid++;
}

// expr => term 
static void parser_expr(parser_t* prs) {
    parser_term_n(prs, 4);
}

// exprlist => expr {',' expr}
static void parser_exprlist(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_expr(prs);
    while (lex->cur.kind == XOC_TOK_COMMA) {
        lexer_next(lex);
        parser_expr(prs);
    }
}

// stmt_assign => designator ( '=' | ':=' | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|=' | '~=' | '<<=' | '>>=' ) expr
static void parser_stmt_assign(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_designator(prs);
        type_t dsg = prs->cur;
        lexer_eat(lex, XOC_TOK_EQ);
        parser_expr(prs);
        parser_push_insts(prs, &(inst_t){
            .op     = XOC_OP_ASSIGN,
            .args   = { [0] = dsg, [1] = prs->cur }
        }, 1);
    }
}

// stmt_assignlist => designatorlist '=' exprlist
static void parser_stmt_assignlist(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_designatorlist(prs);
        lexer_eat(lex, XOC_TOK_EQ);
        parser_exprlist(prs);
    }
}

// simple_stmt => stmt_assign | stmt_assignlist | designator [( '++' | '--' )]
static void parser_stmt_simple(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        parser_stmt_assign(prs);
    // } else if (lex->cur.kind == XOC_TOK_IDT) {
    //     parser_stmt_assignlist(prs);
    } else {
        parser_designator(prs);
        if (lex->cur.kind == XOC_TOK_PLUSPLUS || lex->cur.kind == XOC_TOK_MINUSMINUS) {
            lexer_next(lex);
        }
    }
}

unsigned int parser_add_label(parser_t* prs, char* name) {
    char label[32];
    if (name == NULL) {
        sprintf(label, "_L%d", prs->lid);
    } else {
        sprintf(label, "%s%d", name, prs->lid);
    }
    int len = strlen(label);
    prs->lid++;
    return map_add(prs->syms, label, len + 1);
}

// stmt_if = 'if' expr block [ 'else' ( stmt_if | block ) ]
static void parser_stmt_if(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IF) {
        lexer_eat(lex, XOC_TOK_IF);
        parser_expr(prs);
        inst_t *new_blk = NULL;
        unsigned int new_lbl = parser_add_label(prs, NULL);
        parser_push_insts(prs, &(inst_t){
            .op     = XOC_OP_GOTO_IFN,
            .args   = { [0] = type_lbl(new_lbl), [1] = prs->cur }
        }, 1);
        parser_block(prs);
        if (lex->cur.kind == XOC_TOK_ELSE) {
            lexer_eat(lex, XOC_TOK_ELSE);
            unsigned org_bid = prs->bid, new_bid;
            new_blk = parser_blk_alc(prs, 1);
            new_blk[0].label = new_lbl;
            if (lex->cur.kind == XOC_TOK_IF) {
                parser_stmt_if(prs);
            } else if (lex->cur.kind == XOC_TOK_LBRACE) {
                parser_block(prs);
            }
            // new entry block
            new_lbl = parser_add_label(prs, NULL);
            new_blk = parser_blk_alc(prs, 1);
            new_blk[0].label = new_lbl;
            new_bid = prs->bid;
            parser_blk_swc(prs, org_bid - 1);
            inst_t* org_last = &prs->blk_cur[prs->iid - 1];
            if(org_last->op == XOC_OP_GOTO) {
                org_last->args[0] = type_lbl(new_lbl);
            } else {
                parser_push_insts(prs, &(inst_t){
                    .op     = XOC_OP_GOTO,
                    .args   = { [0] = type_lbl(new_lbl) }
                }, 1);
            }
            parser_blk_swc(prs, new_bid - 1);
        } else {
            new_blk = parser_blk_alc(prs, 1);
            new_blk[0].label = new_lbl;
        }
    }
}

// expr_case => 'case' expr {',' expr} ':' stmtlist
static void parser_expr_case(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_CASE) {
        lexer_eat(lex, XOC_TOK_CASE);
        type_t lhs = prs->cur;
        parser_expr(prs);
        type_t rhs = prs->cur;
        while (lex->cur.kind == XOC_TOK_COMMA) {
            lexer_next(lex);
            parser_expr(prs);
            rhs = prs->cur;
            parser_push_insts(prs, (inst_t[]){{
                    .op     = XOC_OP_BINARY,
                    .args   = { [0] = type_sym(XOC_TOK_EQEQ), [1] = type_tmp(prs->tid), [2] = lhs, [3] = rhs }
                }, {
                    .op     = XOC_OP_BINARY,
                    .args   = { [0] = type_sym(XOC_TOK_OROR), [1] = type_tmp(prs->tid + 1), [2] = lhs, [3] = rhs }
                }
            }, 2);
            prs->tid += 2;
        }
        lexer_eat(lex, XOC_TOK_COLON);
        unsigned new_lbl = parser_add_label(prs, NULL);
        parser_push_insts(prs, &(inst_t){
            .op     = XOC_OP_GOTO_IFNE,
            .args   = { [0] = type_lbl(new_lbl), [1] = lhs, [2] = rhs }
        }, 1);
        parser_stmtlist(prs);
        inst_t* new_blk = parser_blk_alc(prs, 1);
        new_blk[0].label = new_lbl;
    }
}

// stmt_switch => 'switch' [ decl_shortval ';' ] expr '{' { { 'case' expr {',' expr} ':' stmtlist } ['default' ':' stmtlist ] '}'
static void parser_stmt_switch(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_SWITCH) {
        lexer_eat(lex, XOC_TOK_SWITCH);
        parser_expr(prs);
        type_t lhs = prs->cur;
        lexer_eat(lex, XOC_TOK_LBRACE);
        unsigned trg_lbl = parser_add_label(prs, NULL);
        while (lex->cur.kind == XOC_TOK_CASE || lex->cur.kind == XOC_TOK_DEFAULT) {
            int org_bid = prs->bid, new_bid;
            prs->is_break = false;
            if (lex->cur.kind == XOC_TOK_DEFAULT) {
                lexer_eat(lex, XOC_TOK_DEFAULT);
                lexer_eat(lex, XOC_TOK_COLON);
                parser_stmtlist(prs);
            } else if (lex->cur.kind == XOC_TOK_CASE) {
                lexer_eat(lex, XOC_TOK_CASE);
                parser_expr(prs);
                type_t rhs = prs->cur;
                while (lex->cur.kind == XOC_TOK_COMMA) {
                    lexer_next(lex);
                    parser_expr(prs);
                    rhs = prs->cur;
                    parser_push_insts(prs, (inst_t[]){{
                            .op     = XOC_OP_BINARY,
                            .args   = { [0] = type_sym(XOC_TOK_EQEQ), [1] = type_tmp(prs->tid), [2] = lhs, [3] = rhs }
                        }, {
                            .op     = XOC_OP_BINARY,
                            .args   = { [0] = type_sym(XOC_TOK_OROR), [1] = type_tmp(prs->tid + 1), [2] = lhs, [3] = rhs }
                        }
                    }, 2);
                    prs->tid += 2;
                }
                lexer_eat(lex, XOC_TOK_COLON);
                unsigned int new_lbl = parser_add_label(prs, NULL);
                parser_push_insts(prs, &(inst_t){
                    .op     = XOC_OP_GOTO_IFNE,
                    .args   = { [0] = type_lbl(new_lbl), [1] = lhs, [2] = rhs }
                }, 1);
                parser_stmtlist(prs);
                inst_t* new_blk = parser_blk_alc(prs, 1);
                new_blk[0].label = new_lbl;   
            }
            new_bid = prs->bid;
            if(prs->is_break) {
                parser_blk_swc(prs, org_bid - 1);
                parser_push_insts(prs, &(inst_t){
                    .op     = XOC_OP_GOTO,
                    .args   = { [0] = type_lbl(trg_lbl) }
                }, 1);
                parser_blk_swc(prs, new_bid - 1);
            }
        }
        lexer_eat(lex, XOC_TOK_RBRACE);
        inst_t* new_blk = parser_blk_alc(prs, 1);
        new_blk[0].label = trg_lbl;
    }
}

// forheader => [decl_shortval ';'] expr [';' stmt_simple]
static void parser_forheader(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        lexer_eat(lex, XOC_TOK_IDT);
        if (lex->cur.kind == XOC_TOK_SEMICOLON) {
            lexer_eat(lex, XOC_TOK_SEMICOLON);
            parser_expr(prs);
            if (lex->cur.kind == XOC_TOK_SEMICOLON) {
                lexer_eat(lex, XOC_TOK_SEMICOLON);
                parser_stmt_simple(prs);
            }
        }
    }
}

// forinheader => ident [',' ident ['^']] 'in' expr
static void parser_forinheader(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_IDT) {
        lexer_eat(lex, XOC_TOK_IDT);
        if (lex->cur.kind == XOC_TOK_IN) {
            lexer_eat(lex, XOC_TOK_IN);
            parser_expr(prs);
        }
    }
}

// stmt_for => 'for' (forheader | forinheader) block
static void parser_stmt_for(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_FOR) {
        lexer_eat(lex, XOC_TOK_FOR);
        parser_forheader(prs);
        parser_block(prs);
    }
}

// stmt_return => 'return' [exprlist]
static void parser_stmt_return(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_RETURN) {
        lexer_eat(lex, XOC_TOK_RETURN);
        parser_expr(prs);
    }
}

// stmt =>  block | stmt_if | stmt_switch | stmt_for | 'break' | 'continue' | stmt_return | stmt_simple | decl
void parser_stmt(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if(lex->cur.kind == XOC_TOK_LBRACE) {
        parser_block(prs);
    } else if (lex->cur.kind == XOC_TOK_IF) {
        parser_stmt_if(prs);
    } else if (lex->cur.kind == XOC_TOK_SWITCH) {
        parser_stmt_switch(prs);
    } else if (lex->cur.kind == XOC_TOK_FOR) {
        parser_stmt_for(prs);
    } else if (lex->cur.kind == XOC_TOK_BREAK) {
        prs->is_break = true;
        lexer_next(lex);
    } else if (lex->cur.kind == XOC_TOK_CONTINUE) {
        lexer_next(lex);
    } else if (lex->cur.kind == XOC_TOK_RETURN) {
        parser_stmt_return(prs);
    } else {
        parser_stmt_simple(prs);
    }
}

// stmtlist => stmt { ';' stmt }
static void parser_stmtlist(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_stmt(prs);
    while (lex->cur.kind == XOC_TOK_SEMICOLON) {
        lexer_next(lex);
        parser_stmt(prs);
    }
}

// block => '{' stmtlist '}'
static void parser_block(parser_t* prs) {
    lexer_t* lex = prs->lex;
    if (lex->cur.kind == XOC_TOK_LBRACE) {
        lexer_next(lex);
        parser_stmtlist(prs);
        lexer_eat(lex, XOC_TOK_RBRACE);
    }
}

void parser_init(parser_t* prs, lexer_t* lex, pool_t* blks, map_t* syms) {
    prs->tid = 0;
    prs->iid = 0;
    prs->bid = 0;
    prs->lid = 0;
    prs->blks = blks;
    prs->syms = syms;
    prs->lex = lex;
    prs->info = lex->info;
    prs->log = lex->log;

    memset(&prs->symstk, 0, sizeof(prs->symstk));
    prs->symstk_top = -1;

    prs->cur.kind = XOC_TYPE_NONE;
    prs->cur.Ptr  = NULL;

    parser_blk_alc(prs, 1);
}

void parser_free(parser_t* prs) {
    char buf[300];
    int n = 0;
    inst_t* blk = (inst_t*)pool_nat(prs->blks, n);
    printf("\n\n");
    while (blk) {
        printf("\n╭───────────[ID: #%-3d Insts: %4d/%-4d]───────────╮", n, pool_nsize(blk), pool_ncap(blk));
        blk_info(blk, buf, 300, prs->syms);
        printf("\n╰─────────────────────────────────────────────────╯\n");
        blk = (inst_t*)pool_nat(prs->blks, ++n);
    }
}