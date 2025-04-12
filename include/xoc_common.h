/**
 * @file    xoc_common.h
 * @brief   XOC Common Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for common enumerations and structures.
 * @note    Use standard bare C headers for cross-compatibility:
 *          stdint.h, stdbool.h, stddef.h
 */

#ifndef XOC_COMMON_H
#define XOC_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

enum {
    XOC_MAX_STR_LEN     = 255,                          /** Max length of string */
    XOC_MAX_IDT_SIZE    = 256,                          /** Max number of identifiers in list */
    XOC_MAX_MOD_SIZE    = 1024,                         /** Max number of modules */
    XOC_MAX_PAR_SIZE    = 16,                           /** Max number of parameters */
    XOC_MAX_BLK_NEST    = 100,                          /** Max number of block nest */
    XOC_MAX_BLK_GOTO    = 100,                          /** Max number of block goto */
};

typedef enum xoc_identkind {
    XOC_IDT_CONST,                                      /** Identifier: constant */
    XOC_IDT_VAR,                                        /** Identifier: variable */
    XOC_IDT_TYPE,                                       /** Identifier: type */
    XOC_IDT_MOD,                                        /** Identifier: module */
} identkind_t;

typedef enum xoc_typekind {
    XOC_TYPE_NONE,
    XOC_TYPE_FORWARD,
    XOC_TYPE_VOID,
    XOC_TYPE_NULL,        
    XOC_TYPE_INT8,
    XOC_TYPE_INT16,
    XOC_TYPE_INT32,
    XOC_TYPE_INT64,
    XOC_TYPE_UINT8,
    XOC_TYPE_UINT16,
    XOC_TYPE_UINT32,
    XOC_TYPE_UINT64,
    XOC_TYPE_BOOL,
    XOC_TYPE_CHAR,
    XOC_TYPE_REAL32,
    XOC_TYPE_REAL64,
    XOC_TYPE_PTR,
    XOC_TYPE_WPTR,
    XOC_TYPE_ARRAY,
    XOC_TYPE_DYNARRAY,
    XOC_TYPE_STR,
    XOC_TYPE_MAP,
    XOC_TYPE_STRUCT,
    XOC_TYPE_INTERFACE,
    XOC_TYPE_CLOSURE,
    XOC_TYPE_FIBER,
    XOC_TYPE_FN
} typekind_t;

typedef enum xoc_tokenkind {
    XOC_TOK_NONE,
    // -- Keywords
    XOC_TOK_BREAK,
    XOC_TOK_CASE,
    XOC_TOK_CONST,
    XOC_TOK_CONTINUE,
    XOC_TOK_DEFAULT,
    XOC_TOK_ELSE,
    XOC_TOK_ENUM,
    XOC_TOK_FN,
    XOC_TOK_FOR,
    XOC_TOK_IMPORT,
    XOC_TOK_INTERFACE,
    XOC_TOK_IF,
    XOC_TOK_IN,
    XOC_TOK_MAP,
    XOC_TOK_RETURN,
    XOC_TOK_STR,
    XOC_TOK_STRUCT,
    XOC_TOK_SWITCH,
    XOC_TOK_TYPE,
    XOC_TOK_VAR,
    XOC_TOK_WEAK,
    // -- Operators
    XOC_TOK_PLUS,
    XOC_TOK_MINUS,
    XOC_TOK_MUL,
    XOC_TOK_DIV,
    XOC_TOK_MOD,
    XOC_TOK_AND,
    XOC_TOK_OR,
    XOC_TOK_XOR,
    XOC_TOK_SHL,
    XOC_TOK_SHR,
    XOC_TOK_PLUSEQ,
    XOC_TOK_MINUSEQ,
    XOC_TOK_MULEQ,
    XOC_TOK_DIVEQ,
    XOC_TOK_MODEQ,
    XOC_TOK_ANDEQ,
    XOC_TOK_OREQ,
    XOC_TOK_XOREQ,
    XOC_TOK_SHLEQ,
    XOC_TOK_SHREQ,
    XOC_TOK_ANDAND,
    XOC_TOK_OROR,
    XOC_TOK_PLUSPLUS,
    XOC_TOK_MINUSMINUS,
    XOC_TOK_EQEQ,
    XOC_TOK_LESS,
    XOC_TOK_GREATER,
    XOC_TOK_EQ,
    XOC_TOK_QUESTION,
    XOC_TOK_NOT,
    XOC_TOK_NOTEQ,
    XOC_TOK_LESSEQ,
    XOC_TOK_GREATEREQ,
    XOC_TOK_COLONEQ,
    XOC_TOK_LPAR,
    XOC_TOK_RPAR,
    XOC_TOK_LBRACKET,
    XOC_TOK_RBRACKET,
    XOC_TOK_LBRACE,
    XOC_TOK_RBRACE,
    XOC_TOK_CARET,
    XOC_TOK_COMMA,
    XOC_TOK_SEMICOLON,
    XOC_TOK_COLON,
    XOC_TOK_COLONCOLON,
    XOC_TOK_PERIOD,
    XOC_TOK_ELLIPSIS,
    // -- Others
    XOC_TOK_IDT,
    XOC_TOK_INT_LIT,
    XOC_TOK_REAL_LIT,
    XOC_TOK_CHAR_LIT,
    XOC_TOK_STR_LIT,
    XOC_TOK_EOLI,
    XOC_TOK_EOL,
    XOC_TOK_EOF
} tokenkind_t;

typedef enum xoc_opcode {
    XOC_OP_NOP,
    XOC_OP_PUSH,
    XOC_OP_PUSH_ZERO,
    XOC_OP_PUSH_LOCAL_PTR,
    XOC_OP_PUSH_LOCAL_PTR_ZERO,
    XOC_OP_PUSH_LOCAL,
    XOC_OP_PUSH_REG,
    XOC_OP_PUSH_UPVALUE,
    XOC_OP_POP,
    XOC_OP_POP_REG,
    XOC_OP_DUP,
    XOC_OP_SWAP,
    XOC_OP_ZERO,
    XOC_OP_DEREF,
    XOC_OP_ASSIGN,
    XOC_OP_ASSIGN_PARAM,
    XOC_OP_CHANGE_REF_CNT,
    XOC_OP_CHANGE_REF_CNT_GLOBAL,
    XOC_OP_CHANGE_REF_CNT_LOCAL,
    XOC_OP_CHANGE_REF_CNT_ASSIGN,
    XOC_OP_UNARY,
    XOC_OP_BINARY,
    XOC_OP_GET_ARRAY_PTR,
    XOC_OP_GET_DYNARRAY_PTR,
    XOC_OP_GET_MAP_PTR,
    XOC_OP_GET_FIELD_PTR,
    XOC_OP_ASSERT_TYPE,
    XOC_OP_ASSERT_RANGE,
    XOC_OP_WEAKEN_PTR,
    XOC_OP_STRENGTHEN_PTR,
    XOC_OP_GOTO,
    XOC_OP_GOTO_IF,
    XOC_OP_GOTO_IF_NOT,
    XOC_OP_CALL,
    XOC_OP_CALL_INDIRECT,
    XOC_OP_CALL_EXTERN,
    XOC_OP_CALL_BUILTIN,
    XOC_OP_RETURN,
    XOC_OP_ENTER_FRAME,
    XOC_OP_LEAVE_FRAME,
    XOC_OP_HALT
} opcode_t;

enum {
    XOC_NUM_KEYWORD = XOC_TOK_WEAK - XOC_TOK_BREAK + 1, /** Number of keywords */
};

typedef char xoc_identname[XOC_MAX_STR_LEN + 1];        /** XOC identifier name type 0 */
typedef xoc_identname identname_t;                      /** XOC identifier name type 1 */
typedef struct xoc_const const_t;                       /** XOC Type: constant */
typedef struct xoc_inst inst_t;                         /** XOC Instruction */
typedef struct xoc_info info_t;                         /** XOC Information for Debug/Error */
typedef void (*xoc_log_fn)(void* context, const char* fmt, ...);
typedef xoc_log_fn log_fn_t;                            /** XOC Log Function */
typedef struct xoc_log log_t;                           /** XOC Error Information Report */
typedef struct xoc_blob blob_t;                         /** XOC Memory Blob */
typedef struct xoc_pool pool_t;                         /** XOC Memory Pool */
typedef struct xoc_mod mod_t;                           /** XOC Module */
typedef struct xoc_modsrc modsrc_t;                     /** XOC Module Source */
typedef struct xoc_mods mods_t;                         /** XOC Module List */
typedef struct xoc_field field_t;                       /** XOC Type: field */
typedef struct xoc_econst econst_t;                     /** XOC Type: enum constant */
typedef struct xoc_param param_t;                       /** XOC Type: parameter */
typedef struct xoc_func func_t;                         /** XOC Type: signature */
typedef struct xoc_type type_t;                         /** XOC Type: tag type */
typedef struct xoc_ident ident_t;                       /** XOC Type: identifier */
typedef struct xoc_token token_t;                       /** XOC Lexer: Token */
typedef struct xoc_lexer lexer_t;                       /** XOC Lexer: Lexer*/
typedef struct xoc_node node_t;                         /** XOC Parser: Node */
typedef struct xoc_parser parser_t;                     /** XOC Parser: Parser */
typedef struct xoc_compiler_option compiler_option_t;   /** XOC Compiler: Compiler Option */
typedef struct xoc_compiler compiler_t;                 /** XOC Compiler: Compiler */

struct xoc_const {
    union {
        int8_t      Int8;
        uint8_t     UInt8;
        int16_t     Int16;
        uint16_t    UInt16;
        int32_t     Int32;
        uint32_t    UInt32;
        int64_t     Int64;
        uint64_t    UInt64;
        void*       Ptr;
        uint64_t    WPtr;
        float       F32;
        double      F64;
    };
};

struct xoc_type {
    typekind_t kind;
    const_t val;
};

struct xoc_inst {
    opcode_t    op;
    type_t      args[3];
};

struct xoc_info {
    int   row;
    int   pos;
    int   code;
    char* file;
    char* func;
    char* msg;
};

struct xoc_log {
    void* context;
    log_fn_t fmt;
};

struct xoc_blob {
    char* data;
    blob_t* next;
};

struct xoc_pool {
    blob_t* head;
    blob_t* tail;
};

struct xoc_mod {
    bool is_compiled;
    unsigned int key;                                   /** Path Hash */
    void* impl_lib;
    char* alias[XOC_MAX_MOD_SIZE];
    char path[XOC_MAX_STR_LEN + 1];
    char folder[XOC_MAX_STR_LEN + 1];
    char name[XOC_MAX_STR_LEN + 1];
};

struct xoc_modsrc {
    bool is_trusted;
    char* src;
    unsigned int key;                                   /** Path Hash */
    char path[XOC_MAX_STR_LEN + 1];
    char folder[XOC_MAX_STR_LEN + 1];
    char name[XOC_MAX_STR_LEN + 1];
};

struct xoc_mods {
    bool is_enabled;
    int num_mod;
    int num_modsrc;
    char folder[XOC_MAX_STR_LEN + 1];
    mod_t* mod[XOC_MAX_MOD_SIZE];
    modsrc_t* modsrc[XOC_MAX_MOD_SIZE];
    log_t* log;
};

char* xoc_strdup(const char* src);
unsigned int xoc_hash(const char* str);
double xoc_pow(double base, int exp);
bool xoc_isident(char ch);
int xoc_ch2digit(char ch, int base);

void info_init(info_t* info, const char* file, const char* func, int row, int pos, int code);
void info_setpos(info_t* info, const char* file, const char* func, int row, int pos);
void info_setmsg(info_t* info, const char* fmt, va_list args);
void info_free(info_t* info);
void log_fn_info(void* context, const char* fmt, ...);
void log_init(log_t* log, void* context, log_fn_t fn);
void pool_init(pool_t *pool);
void pool_free(pool_t *pool);
char* pool_alc(pool_t *pool, int size);
blob_t* pool_nin(pool_t* pool, char* ptr);
char* pool_nalc(pool_t *pool, int align, int size);
char* pool_nrlc(pool_t* pool, char* ptr, int size);
char* pool_npush(pool_t* pool, char* ptr, char* new, int size);



#endif /* XOC_COMMON_H */