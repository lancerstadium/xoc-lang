#include <xoc_parser.h>
#include <xoc_types.h>

#include <stdio.h>
#include <stdlib.h>


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
static void parser_factor(parser_t* prs);
static void parser_term_n(parser_t* prs, int n);
void parser_expr(parser_t* prs);


static const char* type_mnemonic_tbl[] = {
    [XOC_TYPE_NONE]     = "none",
    [XOC_TYPE_TMP]      = "%",
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

void type_info(type_t* type, char* buf, int len) {
    switch (type->kind) {
        case XOC_TYPE_SYM:  snprintf(buf, len, "%s", lexer_mnemonic(type->WPtr)); break;
        case XOC_TYPE_TMP:  snprintf(buf, len, "%s%ld", type_mnemonic_tbl[type->kind], type->WPtr); break;
        case XOC_TYPE_I64:  snprintf(buf, len, "%ld:%s", type->I64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_F32:  snprintf(buf, len, "%f:%s", type->F32, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_F64:  snprintf(buf, len, "%lf:%s", type->F64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_CHAR: snprintf(buf, len, "'%c':%s", (char)type->I64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_STR:  snprintf(buf, len, "\"%s\":%s", (char*)type->WPtr, type_mnemonic_tbl[type->kind]); break;
        default:            snprintf(buf, len, "%s", type_mnemonic_tbl[type->kind]); break;
    }
}

void inst_info(inst_t* inst, char* buf, int len) {
    char arg[4][64];
    type_info(&inst->args[0], arg[0], 64);
    type_info(&inst->args[1], arg[1], 64);
    type_info(&inst->args[2], arg[2], 64);
    type_info(&inst->args[3], arg[3], 64);
    switch (inst->op) {
        case XOC_OP_PUSH:   snprintf(buf, len, "PUSH   %s := %s", arg[0], arg[1]); break;
        case XOC_OP_UNARY:  snprintf(buf, len, "UNARY  %s := %s %s", arg[1], arg[0], arg[2]); break;
        case XOC_OP_BINARY: snprintf(buf, len, "BINARY %s := %s %s %s", arg[1], arg[2], arg[0], arg[3]); break;
        default:            snprintf(buf, len, "UNKNOW %d", inst->op); break;
    }
}

type_t type_sym(tokenkind_t tk) {
    return (type_t){ .kind = XOC_TYPE_SYM, .WPtr = tk };
}

type_t type_tmp(int id) {
    return (type_t){ .kind = XOC_TYPE_TMP, .WPtr = id };
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
    char* res = pool_npush(prs->blks, (char**)&prs->blk_cur, (char*)insts, size);
    if(!res) {
        prs->log->fmt(prs->info, "Unable to push insts: %p(%u+%d/%u)", insts, pool_nsize(prs->blk_cur), size, pool_ncap(prs->blk_cur));
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
        lexer_next(lex);
        if (lex->cur.kind == XOC_TOK_COLONCOLON) {
            lexer_next(lex);
            lexer_eat(lex, XOC_TOK_IDT);
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
    parser_type(prs);
    lexer_eat(lex, XOC_TOK_LPAR);
    parser_expr(prs);
    lexer_eat(lex, XOC_TOK_RPAR);
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
    // parser_block(prs);
}

// compositelit => [type] (veclit | maplit | structlit | closurelit)
static void parser_compositelit(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_type(prs);

}

// enumconst => [type] '.' ident
static void parser_enumconst(parser_t* prs) {
    lexer_t* lex = prs->lex;
    parser_type(prs);
    lexer_eat(lex, XOC_TOK_PERIOD);
    lexer_eat(lex, XOC_TOK_IDT);
}

// designator => ( primary | typecast | compositelit | enumconst ) selectors
static void parser_designator(parser_t* prs) {
    lexer_t* lex = prs->lex;
    
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

// decl_assign => ident ':=' expr

// decl_assignlist => identlist ':=' exprlist

// decl_shortval => decl_assign | decl_assignlist

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
            case XOC_TOK_STR_LIT: prs->cur = (type_t){ .kind = XOC_TYPE_WPTR, .WPtr = lex->cur.key }; break;
            case XOC_TOK_IDT: prs->cur = (type_t){ .kind = XOC_TYPE_WPTR, .WPtr = lex->cur.key }; break;
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
void parser_expr(parser_t* prs) {
    parser_term_n(prs, 4);
}



// stmt_assign => designator ( ':=' | '+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|=' | '~=' | '<<=' | '>>=' ) expr

// stmt_assignlist => designatorlist '=' exprlist

// simple_stmt => stmt_assign | stmt_assignlist | designator [( '++' | '--' )]

// stmt_if = 'if' [ decl_shortval ';' ] expr block [ 'else' ( stmt_if | block ) ]

// expr_case => 'case' expr {',' expr} ':' stmtlist

// stmt_switch => 'switch' [ decl_shortval ';' ] expr '{' { { expr_case } ['default' : stmtlist ] '}'

// forheader => [decl_shortval ';'] expr [';' stmt_simple]

// forinheader => ident [',' ident ['^']] 'in' expr

// stmt_for => 'for' (forheader | forinheader) block

// stmt_return => 'return' [exprlist]

// stmt => decl | block | stmt_simple | stmt_if | stmt_switch | stmt_for | 'break' | 'continue' | stmt_return
void parser_stmt(parser_t* prs) {
    // parse a statement
    // lexer_t* lex = prs->lex;
    char buf[300];
    prs->blk_cur = (inst_t*)pool_nalc(prs->blks, sizeof(inst_t), 1);
    prs->log->fmt(prs->info, "insts: %d/%d", pool_nsize(prs->blk_cur), pool_ncap(prs->blk_cur));
    parser_expr(prs);
    prs->log->fmt(prs->info, "insts: %d/%d", pool_nsize(prs->blk_cur), pool_ncap(prs->blk_cur));
    for (int i = 0; i < pool_nsize(prs->blk_cur); i++) {
        inst_info(&prs->blk_cur[i], buf, 300);
        printf("\n%s", buf);
    }
}

// stmtlist => stmt { ';' stmt }

// block => '{' stmtlist '}'

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