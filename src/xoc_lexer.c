#include <xoc_lexer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

static const char* token_mnemonic_tbl[] = {
    [XOC_TOK_NONE]      = "none",
    // -- Keywords
    [XOC_TOK_BREAK]     = "break",
    [XOC_TOK_CASE]      = "case",
    [XOC_TOK_CONST]     = "const",
    [XOC_TOK_CONTINUE]  = "continue",
    [XOC_TOK_DEFAULT]   = "default",
    [XOC_TOK_ELSE]      = "else",
    [XOC_TOK_ENUM]      = "enum",
    [XOC_TOK_FN]        = "fn",
    [XOC_TOK_FOR]       = "for",
    [XOC_TOK_IMPORT]    = "import",
    [XOC_TOK_INTERFACE] = "interface",
    [XOC_TOK_IF]        = "if",
    [XOC_TOK_IN]        = "in",
    [XOC_TOK_MAP]       = "map",
    [XOC_TOK_RETURN]    = "return",
    [XOC_TOK_STR]       = "str",
    [XOC_TOK_STRUCT]    = "struct",
    [XOC_TOK_SWITCH]    = "switch",
    [XOC_TOK_TYPE]      = "type",
    [XOC_TOK_VAR]       = "var",
    [XOC_TOK_WEAK]      = "weak",
    // -- Operators
    [XOC_TOK_PLUS]      = "+",
    [XOC_TOK_MINUS]     = "-",
    [XOC_TOK_MUL]       = "*",
    [XOC_TOK_DIV]       = "/",
    [XOC_TOK_MOD]       = "%",
    [XOC_TOK_AND]       = "&",
    [XOC_TOK_OR]        = "|",
    [XOC_TOK_XOR]       = "~",
    [XOC_TOK_SHL]       = "<<",
    [XOC_TOK_SHR]       = ">>",
    [XOC_TOK_PLUSEQ]    = "+=",
    [XOC_TOK_MINUSEQ]   = "-=",
    [XOC_TOK_MULEQ]     = "*=",
    [XOC_TOK_DIVEQ]     = "/=",
    [XOC_TOK_MODEQ]     = "%=",
    [XOC_TOK_ANDEQ]     = "&=",
    [XOC_TOK_OREQ]      = "|=",
    [XOC_TOK_XOREQ]     = "~=",
    [XOC_TOK_SHLEQ]     = "<<=",
    [XOC_TOK_SHREQ]     = ">>=",
    [XOC_TOK_ANDAND]    = "&&",
    [XOC_TOK_OROR]      = "||",
    [XOC_TOK_PLUSPLUS]  = "++",
    [XOC_TOK_MINUSMINUS]= "--",
    [XOC_TOK_EQEQ]      = "==",
    [XOC_TOK_LESS]      = "<",
    [XOC_TOK_GREATER]   = ">",
    [XOC_TOK_EQ]        = "=",
    [XOC_TOK_QUESTION]  = "?",
    [XOC_TOK_NOT]       = "!",
    [XOC_TOK_NOTEQ]     = "!=",
    [XOC_TOK_LESSEQ]    = "<=",
    [XOC_TOK_GREATEREQ] = ">=",
    [XOC_TOK_COLONEQ]   = ":=",
    [XOC_TOK_LPAR]      = "(",
    [XOC_TOK_RPAR]      = ")",
    [XOC_TOK_LBRACKET]  = "[",
    [XOC_TOK_RBRACKET]  = "]",
    [XOC_TOK_LBRACE]    = "{",
    [XOC_TOK_RBRACE]    = "}",
    [XOC_TOK_CARET]     = "^",
    [XOC_TOK_COMMA]     = ",",
    [XOC_TOK_SEMICOLON] = ";",
    [XOC_TOK_COLON]     = ":",
    [XOC_TOK_COLONCOLON]= "::",
    [XOC_TOK_PERIOD]    = ".",
    [XOC_TOK_ELLIPSIS]  = "..",
    // -- Others
    [XOC_TOK_IDT]       = "ident",
    [XOC_TOK_INT_LIT]   = "int",
    [XOC_TOK_REAL_LIT]  = "real",
    [XOC_TOK_CHAR_LIT]  = "char",
    [XOC_TOK_STR_LIT]   = "str",
    [XOC_TOK_EOLI]      = "EOLI",
    [XOC_TOK_EOL]       = "EOL",
    [XOC_TOK_EOF]       = "EOF",
};

static unsigned int keyword_hash[XOC_NUM_KEYWORD];  /** Keyword hash */
static inline char lexer_getc(lexer_t* lex);
static inline bool lexer_getceq(lexer_t* lex, char ch);
static inline char lexer_escc(lexer_t* lex, bool* escaped);

static inline char lexer_getc(lexer_t* lex) {
    char ch = lex->buf[lex->buf_pos];
    if(ch) {
        lex->buf_pos++;
        lex->pos++;
        if(ch == '\n') {
            lex->row++;
            lex->pos = 1;
        }
    }
    return ch;
}

static inline bool lexer_getceq(lexer_t* lex, char ch) {
    if(lex->buf[lex->buf_pos] == ch) {
        lexer_getc(lex);
        return true;
    }
    return false;
}


static inline char lexer_escc(lexer_t* lex, bool* escaped) {
    if(escaped) *escaped = false;
    char ch = lexer_getc(lex);
    if(ch == '\\') {
        ch = lexer_getc(lex);
        if(escaped) *escaped = true;
        switch(ch) {
            case '0': return '\0';
            case 'a': return '\a';
            case 'b': return '\b';
            case 'f': return '\f';
            case 'n': return '\n';
            case 'r': return '\r';
            case 't': return '\t';
            case 'v': return '\v';
            case 'x': {
                lexer_getc(lex);
                unsigned int hex = 0;
                int len = 0;
                const int items = sscanf(lex->buf + lex->buf_pos, "%x%n", &hex, &len);
                if(items < 1 || hex > 0xFF) {
                    lex->log->fmt(lex->log->context, "Invalid character code: \\x%x`", hex);
                    lex->cur.kind = XOC_TOK_NONE;
                    return '\0';
                }
                lex->buf_pos += len - 1;
                lex->pos += len - 1;
                return (char)hex;
            }
            default: return ch;
        }
    }
    return ch;
}

static inline void lexer_slcmt(lexer_t* lex) {
    char ch = lexer_getc(lex);
    while(ch && ch != '\n') {
        ch = lexer_getc(lex);
    }
}

static inline void lexer_mlcmt(lexer_t* lex) {
    char ch = lexer_getc(lex);
    bool asterisk = false;
    while(ch && !(ch == '/' && asterisk)) {
        asterisk = false;
        while(ch && ch != '*') {
            ch = lexer_getc(lex);
        }
        if(ch == '*') {
            asterisk = true;
        }
        ch = lexer_getc(lex);
    }
    ch = lexer_getc(lex);
}

static inline void lexer_spcmt(lexer_t* lex) {
    char ch = lex->buf[lex->buf_pos];
    while (ch && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '/')) {
        if(ch == '/') {
            lexer_getc(lex);
            if(lexer_getceq(lex, '/')) {
                lexer_slcmt(lex);
            } else if(lexer_getceq(lex, '*')) {
                lexer_mlcmt(lex);
            } else {
                lex->buf_pos--;
                lex->pos--;
                break;
            }
            ch = lex->buf[lex->buf_pos];
        } else {
            lexer_getc(lex);
            ch = lex->buf[lex->buf_pos];
        }

    }
}


static inline void lexer_keyidt(lexer_t* lex) {
    lex->cur.kind = XOC_TOK_NONE;
    char ch = lex->buf[lex->buf_pos];
    int len = 0;
    identname_t name;
    // Get identifier name
    while (xoc_isident(ch)) {
        name[len] = ch;
        len++;
        lexer_getc(lex);
        if(len > XOC_MAX_STR_LEN) {
            lex->log->fmt(lex->info, "Identifier name is too long: %s", name);
            lex->cur.kind = XOC_TOK_NONE;
            return;
        }
        ch = lex->buf[lex->buf_pos];
    }
    name[len] = '\0';
    lex->cur.key = xoc_hash(name);
    map_add(lex->syms, name, len);
    // lex->log->fmt(lex->info, "ident: %s, key: %08x", lex->cur.name, lex->cur.key);
    
    // Search keyword
    for (int i = 0; i < XOC_NUM_KEYWORD; i++) {
        if(keyword_hash[i] == lex->cur.key && strcmp(name, token_mnemonic_tbl[XOC_TOK_BREAK + i]) == 0) {
            lex->cur.kind = XOC_TOK_BREAK + i;
            return;
        }
    }

    // Identifier
    if(lex->cur.kind == XOC_TOK_NONE) {
        lex->cur.kind = XOC_TOK_IDT;
    }
}

static inline void lexer_ops(lexer_t* lex) {
    lex->cur.kind = XOC_TOK_NONE;
    char ch = lexer_getc(lex);

    switch(ch) {
        case '+': 
            if(lexer_getceq(lex, '+')) {
                lex->cur.kind = XOC_TOK_PLUSPLUS;
            } else if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_PLUSEQ;
            } else {
                lex->cur.kind = XOC_TOK_PLUS;
            }
            break;
        case '-':
            if(lexer_getceq(lex, '-')) {
                lex->cur.kind = XOC_TOK_MINUSMINUS;
            } else if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_MINUSEQ;
            } else {
                lex->cur.kind = XOC_TOK_MINUS;
            }
            break;
        case '*':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_MULEQ;
            } else {
                lex->cur.kind = XOC_TOK_MUL;
            }
            break;
        case '/':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_DIVEQ;
            } else {
                lex->cur.kind = XOC_TOK_DIV;
            }
            break;
        case '%':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_MODEQ;
            } else {
                lex->cur.kind = XOC_TOK_MOD;
            }
            break;
        case '&':
            if(lexer_getceq(lex, '&')) {
                lex->cur.kind = XOC_TOK_ANDAND;
            } else if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_ANDEQ;
            } else {
                lex->cur.kind = XOC_TOK_AND;
            }
            break;
        case '|':
            if(lexer_getceq(lex, '|')) {
                lex->cur.kind = XOC_TOK_OROR;
            } else if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_OREQ;
            } else {
                lex->cur.kind = XOC_TOK_OR;
            }
            break;
        case '~':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_XOREQ;
            } else {
                lex->cur.kind = XOC_TOK_XOR;
            }
            break;
        case '<':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_LESSEQ;
            } else if(lexer_getceq(lex, '<')) {
                if (lexer_getceq(lex, '=')) {
                    lex->cur.kind = XOC_TOK_SHLEQ;
                } else {
                    lex->cur.kind = XOC_TOK_SHL;
                }
            } else {
                lex->cur.kind = XOC_TOK_LESS;
            }
            break;
        case '>':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_GREATEREQ;
            } else if(lexer_getceq(lex, '>')) {
                if (lexer_getceq(lex, '=')) {
                    lex->cur.kind = XOC_TOK_SHREQ;
                } else {
                    lex->cur.kind = XOC_TOK_SHR;
                }
            } else {
                lex->cur.kind = XOC_TOK_GREATER;
            }
            break;
        case '=':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_EQEQ;
            } else {
                lex->cur.kind = XOC_TOK_EQ;
            }
            break;
        case '!':
            if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_NOTEQ;
            } else {
                lex->cur.kind = XOC_TOK_NOT;
            }
            break;
        case ':':
            if(lexer_getceq(lex, ':')) {
                lex->cur.kind = XOC_TOK_COLONCOLON;
            } else if(lexer_getceq(lex, '=')) {
                lex->cur.kind = XOC_TOK_COLONEQ;
            } else {
                lex->cur.kind = XOC_TOK_COLON;
            }
            break;
        case '.':
            if(lexer_getceq(lex, '.')) {
                lex->cur.kind = XOC_TOK_ELLIPSIS;
            } else {
                lex->cur.kind = XOC_TOK_PERIOD;
            }
            break;
        case '?': lex->cur.kind = XOC_TOK_QUESTION; break;
        case '(': lex->cur.kind = XOC_TOK_LPAR; break;
        case ')': lex->cur.kind = XOC_TOK_RPAR; break;
        case '[': lex->cur.kind = XOC_TOK_LBRACKET; break;
        case ']': lex->cur.kind = XOC_TOK_RBRACKET; break;
        case '{': lex->cur.kind = XOC_TOK_LBRACE; break;
        case '}': lex->cur.kind = XOC_TOK_RBRACE; break;
        case '^': lex->cur.kind = XOC_TOK_CARET; break;
        case ',': lex->cur.kind = XOC_TOK_COMMA; break;
        case ';': lex->cur.kind = XOC_TOK_SEMICOLON; break;
        case '\n': lex->cur.kind = XOC_TOK_EOL; break;
        default: break;
    }
}

static inline uint64_t lexer_digitlit(lexer_t* lex, int base, int* len, bool is_frac) {
    uint64_t res = 0;
    *len = 0;
    if(xoc_ch2digit(lex->buf[lex->buf_pos], base) == -1) {
        lex->log->fmt(lex->log->context, "Expect digit, but got `%c`", lex->buf[lex->buf_pos]);
        return res;
    }
    bool is_skip = false;
    while(xoc_ch2digit(lex->buf[lex->buf_pos], base) != -1) {
        uint64_t new_res = res * base + xoc_ch2digit(lex->buf[lex->buf_pos], base);
        if ((res * base) / base != res || new_res < res) {
            if(is_frac) {
                is_skip = true;
            } else {
                lex->log->fmt(lex->log->context, "Number literal too large");
            }
        }
        if(!is_skip) {
            res = new_res;
            (*len)++;
        }
        lexer_getc(lex);
        if(lex->buf[lex->buf_pos] == '_') {
            if(xoc_ch2digit(lex->buf[lex->buf_pos + 1], base) == -1) {
                lex->log->fmt(lex->log->context, "Expect digit after `_`, but got `%c`", lex->buf[lex->buf_pos + 1]);
            } else {
                lexer_getc(lex);
            }
        }
    }
    return res;
}

static void lexer_numlit(lexer_t* lex) {
    lex->cur.kind = XOC_TOK_NONE;
    uint64_t whole = 0, frac = 0, expon = 0;
    bool is_expneg = false, is_real = false;
    int base = 10, whole_len = 0, frac_len = 0, expon_len = 0;
    if ((lex->buf[lex->buf_pos] == '0') && (lex->buf[lex->buf_pos + 1] == 'x' || lex->buf[lex->buf_pos + 1] == 'X')) {
        lexer_getc(lex);
        lexer_getc(lex);
        lexer_getceq(lex, '_');
        base = 16;
    }
    if (lex->buf[lex->buf_pos] == '.' && xoc_ch2digit(lex->buf[lex->buf_pos + 1], 10) == -1) {
        return;
    }
    if (!(lex->buf[lex->buf_pos] == '.' && base == 10)) {
        whole = lexer_digitlit(lex, base, &whole_len, false);
    }
    if (base == 10) {
        if(lexer_getceq(lex, '.')) {
            is_real = true;
            if (xoc_ch2digit(lex->buf[lex->buf_pos], 10) != -1) {
                frac = lexer_digitlit(lex, 10, &frac_len, true);
            }
        }
        if (lexer_getceq(lex, 'e') || lexer_getceq(lex, 'E')) {
            is_real = true;
            if (lexer_getceq(lex, '-')) {
                is_expneg = true;
            } else {
                lexer_getceq(lex, '+');
            }
            expon = lexer_digitlit(lex, 10, &expon_len, false);
        }
    }
    if (is_real) {
        lex->cur.kind = XOC_TOK_REAL_LIT;
        lex->cur.Real = (double)whole + (double)frac / (double)xoc_pow(10, frac_len);
        if (is_expneg) {
            lex->cur.Real /= xoc_pow(10, expon);
        } else {
            lex->cur.Real *= xoc_pow(10, expon);
        }
        if (lex->cur.Real < -DBL_MAX || lex->cur.Real > DBL_MAX) {
            lex->log->fmt(NULL, "Number literal too large");
        }
    } else {
        lex->cur.kind = XOC_TOK_INT_LIT;
        lex->cur.Int = whole;
    }
}

static inline void lexer_chlit(lexer_t* lex) {
    lexer_getc(lex);
    lex->cur.kind = XOC_TOK_CHAR_LIT;
    lex->cur.Int = lexer_escc(lex, NULL);
    const char ch = lexer_getc(lex);
    if (ch != '\'') {
        lex->log->fmt(NULL, "Expect char literal, but got `%c`", ch);
        lex->cur.kind = XOC_TOK_NONE;
    }
    lexer_getc(lex);
}

static inline int lexer_strslit(lexer_t* lex) {
    lex->cur.kind = XOC_TOK_STR_LIT;
    int len = 0;
    bool escaped = false;
    char ch = lexer_escc(lex, &escaped);
    while (ch != '\"' || escaped) {
        if (ch == '\0' || (ch == '\n' && !escaped)) {
            lex->log->fmt(NULL, "Untterminated string literal");
            lex->cur.kind = XOC_TOK_NONE;
            return len;
        }
        if (lex->cur.Str) {
            lex->cur.Str[len] = ch;
        }
        len++;
        ch = lexer_escc(lex, &escaped);
    }
    if(lex->cur.Str) {
        lex->cur.Str[len] = '\0';   
    }
    ch = lexer_getc(lex);
    return len;
}

static inline void lexer_strlit(lexer_t* lex) {
    lexer_getc(lex);
    lexer_t aheadlex = *lex;
    aheadlex.cur.Str = NULL;
    int len = lexer_strslit(&aheadlex);
    lex->cur.Str = pool_alc(lex->pool, len + 1);
    lexer_strslit(lex);
}

static inline void lexer_next_eol(lexer_t* lex) {

    lexer_spcmt(lex);
    lex->cur.kind = XOC_TOK_NONE;
    lex->cur.row = lex->info->row = lex->row;
    lex->cur.pos = lex->info->pos = lex->pos;
    char ch = lex->buf[lex->buf_pos];
    // lex->log->fmt(lex->info, "now `%c`", ch);
    if((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_') {
        lexer_keyidt(lex);
    } else if (ch >= '0' && ch <= '9') {
        lexer_numlit(lex);
    } else if (ch == '\'') {
        lexer_chlit(lex);
    } else if (ch == '"') {
        lexer_strlit(lex);
    } else {
        lexer_ops(lex);
    }

    if (lex->cur.kind == XOC_TOK_NONE) {
        if(!lex->buf[lex->buf_pos]) {
            lex->cur.kind = XOC_TOK_EOF;
        } else {
            lex->log->fmt(lex->info, "Unexpected character `%c` or end of file.", ch);
        }
    }
}



int lexer_init(lexer_t* lex, const char* src, bool trusted, pool_t* pool, map_t* syms, info_t* info, log_t* log) {
    // 1. Fill keyword hash
    for (int i = 0; i < XOC_NUM_KEYWORD; i++) {
        keyword_hash[i] = xoc_hash(token_mnemonic_tbl[XOC_TOK_BREAK + i]);
    }

    // 2. Read source file/buffer
    int buf_len = 0;
    lex->buf        = xoc_strdup(src);
    buf_len         = lex->buf ? strlen(lex->buf) : 0;

    // 3. Initialize lexer
    lex->pool       = pool;
    lex->syms       = syms;
    lex->info       = info;
    lex->log        = log;
    lex->is_trusted = trusted;
    lex->buf_pos    = 0;
    lex->row        = 1;
    lex->pos        = 1;
    lex->cur.kind   = XOC_TOK_NONE;
    lex->cur.Str    = NULL;
    lex->cur.row    = lex->row;
    lex->cur.pos    = lex->pos;
    lex->prev       = lex->cur;

    return buf_len;
}

void lexer_free(lexer_t* lex) {
    // 如果缓冲区不为空
    if(lex->buf) {
        // 释放缓冲区
        free(lex->buf);
        // 将缓冲区指针置为空
        lex->buf = NULL;
    }
}

void token_info(token_t* tok, char* buf, int len, map_t* syms) {
    switch (tok->kind) {
        case XOC_TOK_IDT        : snprintf(buf, len, "<'%s':`%s`>", map_get(syms, tok->key), token_mnemonic_tbl[tok->kind]);   break;
        case XOC_TOK_CHAR_LIT   : snprintf(buf, len, "<'%c':`%s`>" , (char)tok->Int  , token_mnemonic_tbl[tok->kind]);  break;
        case XOC_TOK_STR_LIT    : snprintf(buf, len, "<\"%s\":`%s`>" , tok->Str , token_mnemonic_tbl[tok->kind]); break;
        case XOC_TOK_INT_LIT    : snprintf(buf, len, "<%ld:`%s`>" , tok->Int  , token_mnemonic_tbl[tok->kind]);   break;
        case XOC_TOK_REAL_LIT   : snprintf(buf, len, "<%f:`%s`>"  , tok->Real , token_mnemonic_tbl[tok->kind]);   break;
        default                 : snprintf(buf, len, "<`%s`>"     , token_mnemonic_tbl[tok->kind]);               break;
    }
}

void lexer_next(lexer_t* lex) {
    if(!lex->buf) return;
    char msg_buf[300];
    do {
        lexer_next_eol(lex);
        if (lex->cur.kind == XOC_TOK_EOL) {
            if( lex->prev.kind == XOC_TOK_BREAK       ||
                lex->prev.kind == XOC_TOK_CONTINUE    ||
                lex->prev.kind == XOC_TOK_RETURN      ||
                lex->prev.kind == XOC_TOK_STR         ||
                lex->prev.kind == XOC_TOK_PLUSPLUS    ||
                lex->prev.kind == XOC_TOK_MINUSMINUS  ||
                lex->prev.kind == XOC_TOK_RPAR        ||
                lex->prev.kind == XOC_TOK_RBRACKET    ||
                lex->prev.kind == XOC_TOK_RBRACE      ||
                lex->prev.kind == XOC_TOK_CARET       ||
                lex->prev.kind == XOC_TOK_IDT         ||
                lex->prev.kind == XOC_TOK_INT_LIT     ||
                lex->prev.kind == XOC_TOK_REAL_LIT    ||
                lex->prev.kind == XOC_TOK_CHAR_LIT    ||
                lex->prev.kind == XOC_TOK_STR_LIT) {
                lex->cur.kind = XOC_TOK_EOLI;
            }
            lex->cur.kind = XOC_TOK_EOLI;
        }
        lex->prev = lex->cur;
        token_info(&lex->cur, msg_buf, 300, lex->syms);
        lex->log->fmt(lex->info, "%s", msg_buf);
    } while (lex->cur.kind == XOC_TOK_EOL);
}

void lexer_nextf(lexer_t* lex) {
    lexer_next_eol(lex);
    // Replace eol with impl_semicol
    if (lex->cur.kind == XOC_TOK_EOL) {
        lex->cur.kind = XOC_TOK_EOLI;
    }
    lex->prev = lex->cur;
}

bool lexer_check(lexer_t* lex, tokenkind_t kind) {
    bool res = lex->cur.kind == kind;
    if(!res) {
        lex->log->fmt(lex->info, "Expect token `%s`, but got `%s`", token_mnemonic_tbl[kind], token_mnemonic_tbl[lex->cur.kind]);
    }
    return res;
}

void lexer_eat(lexer_t* lex, tokenkind_t kind) { 
    if (!(kind == XOC_TOK_SEMICOLON && (lex->cur.kind == XOC_TOK_RPAR || lex->cur.kind == XOC_TOK_RBRACKET || lex->cur.kind == XOC_TOK_RBRACE))) {
        lexer_check(lex, kind);
        lexer_next(lex);
    }
}


const char* lexer_mnemonic(tokenkind_t kind) {
    return token_mnemonic_tbl[kind];
}

tokenkind_t lexer_trans_assign(tokenkind_t kind) {
    // Full replacements for short assignment operators
    switch (kind) {
        case XOC_TOK_PLUSEQ:    return XOC_TOK_PLUS;
        case XOC_TOK_MINUSEQ:   return XOC_TOK_MINUS;
        case XOC_TOK_MULEQ:     return XOC_TOK_MUL;
        case XOC_TOK_DIVEQ:     return XOC_TOK_DIV;
        case XOC_TOK_MODEQ:     return XOC_TOK_MOD;
        case XOC_TOK_ANDEQ:     return XOC_TOK_AND;
        case XOC_TOK_OREQ:      return XOC_TOK_OR;
        case XOC_TOK_XOREQ:     return XOC_TOK_XOR;
        case XOC_TOK_SHLEQ:     return XOC_TOK_SHL;
        case XOC_TOK_SHREQ:     return XOC_TOK_SHR;
        default:                return XOC_TOK_NONE;
    }
}