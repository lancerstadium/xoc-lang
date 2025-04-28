#include <xoc_types.h>
#include <xoc_lexer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* type_mnemonic_tbl[] = {
    [XOC_TYPE_NONE]     = "none",
    [XOC_TYPE_TMP]      = "$",
    [XOC_TYPE_IDT]      = "$",
    [XOC_TYPE_BLK]      = "#",
    [XOC_TYPE_LBL]      = "%",
    [XOC_TYPE_DVC]      = "@",
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
    [XOC_TYPE_DYNARRAY] = "[..]",
    [XOC_TYPE_ARRAY]    = "[]",
    [XOC_TYPE_MAP]      = "map",
};

static const char* device_mnemonic_tbl[] = {
    [XOC_DVC_NONE]      = "none",
    [XOC_DVC_CPU]       = "cpu",
    [XOC_DVC_GPU]       = "gpu",
};


type_t* type_alc(typekind_t kind) {
    type_t* type = (type_t*)malloc(sizeof(type_t));
    if (!type) {
        return NULL;
    }
    memset(type, 0, sizeof(type_t));
    type->kind = kind;
    return type;
}

void type_free(type_t* type) {
    if (!type) {
        return;
    }
    while (type->next) {
        type_t* next = type->next;
        if (type->base) {
            free(type->base);
        }
        free(type);
        type = next;
    }
    free(type);
    type = NULL;
}

type_t* type_i8(int8_t i8) {
    type_t* type = type_alc(XOC_TYPE_I8);
    type->val.I8 = i8;
    return type;
}

type_t* type_u8(uint8_t u8) {
    type_t* type = type_alc(XOC_TYPE_U8);
    type->val.U8 = u8;
    return type;
}

type_t* type_i16(int16_t i16) {
    type_t* type = type_alc(XOC_TYPE_I16);
    type->val.I16 = i16;
    return type;
}
type_t* type_u16(uint16_t u16) {
    type_t* type = type_alc(XOC_TYPE_U16);
    type->val.U16 = u16;
    return type;
}

type_t* type_i32(int32_t i32) {
    type_t* type = type_alc(XOC_TYPE_I32);
    type->val.I32 = i32;
    return type;
}

type_t* type_u32(uint32_t u32) {
    type_t* type = type_alc(XOC_TYPE_U32);
    type->val.U32 = u32;
    return type;
}

type_t* type_i64(int64_t i64) {
    type_t* type = type_alc(XOC_TYPE_I64);
    type->val.I64 = i64;
    return type;
}

type_t* type_u64(uint64_t u64) {
    type_t* type = type_alc(XOC_TYPE_U64);
    type->val.U64 = u64;
    return type;
}

type_t* type_f32(float f32) {
    type_t* type = type_alc(XOC_TYPE_F32);
    type->val.F32 = f32;
    return type;
}

type_t* type_f64(double f64) {
    type_t* type = type_alc(XOC_TYPE_F64);
    type->val.F64 = f64;
    return type;
}

type_t* type_char(char c) {
    type_t* type = type_alc(XOC_TYPE_CHAR);
    type->val.I64 = c;
    return type;
}

type_t* type_str(uint64_t key) {
    type_t* type = type_alc(XOC_TYPE_STR);
    type->val.WPtr = key;
    return type;
}

type_t* type_tok(tokenkind_t tk) {
    type_t* type = type_alc(XOC_TYPE_TOK);
    type->val.WPtr = tk;
    return type;
}

type_t* type_tmp(int id) {
    type_t* type = type_alc(XOC_TYPE_TMP);
    type->val.WPtr = id;
    return type;
}

type_t* type_idt(uint64_t key) {
    type_t* type = type_alc(XOC_TYPE_IDT);
    type->val.WPtr = key;
    return type;
}

type_t* type_blk(int id) {
    type_t* type = type_alc(XOC_TYPE_BLK);
    type->val.WPtr = id;
    return type;
}

type_t* type_lbl(uint64_t key) {
    type_t* type = type_alc(XOC_TYPE_LBL);
    type->val.WPtr = key;
    return type;
}

type_t* type_dvc(devicekind_t dvc) {
    type_t* type = type_alc(XOC_TYPE_DVC);
    type->val.WPtr = dvc;
    return type;
}

int type_size(type_t* type) {
    switch (type->kind) {
        case XOC_TYPE_VOID: return 0;
        case XOC_TYPE_BOOL: return sizeof(bool);
        case XOC_TYPE_I8:   return sizeof(int8_t);
        case XOC_TYPE_U8:   return sizeof(uint8_t);
        case XOC_TYPE_I16:  return sizeof(int16_t);
        case XOC_TYPE_U16:  return sizeof(uint16_t);
        case XOC_TYPE_I32:  return sizeof(int32_t);
        case XOC_TYPE_U32:  return sizeof(uint32_t);
        case XOC_TYPE_I64:  return sizeof(int64_t);
        case XOC_TYPE_U64:  return sizeof(uint64_t);
        case XOC_TYPE_F32:  return sizeof(float);
        case XOC_TYPE_F64:  return sizeof(double);
        case XOC_TYPE_CHAR: return sizeof(unsigned char);
        case XOC_TYPE_STR:  return sizeof(char*);
        case XOC_TYPE_PTR:  return sizeof(void*);
        case XOC_TYPE_WPTR: return sizeof(uint64_t);
        case XOC_TYPE_STRUCT: 
        case XOC_TYPE_INTERFACE: 
        case XOC_TYPE_CLOSURE: {
        
        }
        default:
            return -1;
    };
}

void type_info(type_t* type, char* buf, int len, map_t* syms) {
    if (!type) return;
    switch (type->kind) {
        case XOC_TYPE_TOK:      snprintf(buf, len, "%s", lexer_mnemonic(type->val.WPtr)); break;
        case XOC_TYPE_TMP:      snprintf(buf, len, "%s%ld", type_mnemonic_tbl[type->kind], type->val.WPtr); break;
        case XOC_TYPE_IDT:      snprintf(buf, len, "%s%s", type_mnemonic_tbl[type->kind], map_get(syms, type->val.WPtr)); break;
        case XOC_TYPE_BLK:      snprintf(buf, len, "%s%ld", type_mnemonic_tbl[type->kind], type->val.WPtr); break;
        case XOC_TYPE_LBL:      snprintf(buf, len, "%s%s", type_mnemonic_tbl[type->kind], map_get(syms, type->val.WPtr)); break;
        case XOC_TYPE_DVC:      snprintf(buf, len, "%s%s", type_mnemonic_tbl[type->kind], device_mnemonic_tbl[type->val.WPtr]); break;
        case XOC_TYPE_I64:      snprintf(buf, len, "%ld:%s", type->val.I64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_F32:      snprintf(buf, len, "%f:%s", type->val.F32, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_F64:      snprintf(buf, len, "%lf:%s", type->val.F64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_CHAR:     snprintf(buf, len, "'%c':%s", (char)type->val.I64, type_mnemonic_tbl[type->kind]); break;
        case XOC_TYPE_STR:      snprintf(buf, len, "\"%s\":%s", (char*)type->val.WPtr, type_mnemonic_tbl[type->kind]); break;
        default:                snprintf(buf, len, "%s", type_mnemonic_tbl[type->kind]); break;
    }
}

void inst_info(inst_t* inst, char* buf, int len, map_t* syms) {
    char label[64];
    char opr[4][64];
    if(inst->label != 0) {
        snprintf(label, 64, " %%%-46s │\n│", map_get(syms, inst->label));
    } else {
        label[0] = 0;
    }
    type_info(inst->opr[0], opr[0], 64, syms);
    type_info(inst->opr[1], opr[1], 64, syms);
    type_info(inst->opr[2], opr[2], 64, syms);
    type_info(inst->opr[3], opr[3], 64, syms);
    switch (inst->opc) {
        case XOC_OP_REG:        snprintf(buf, len, "%s  REG        %s %s"           , label, opr[0], opr[1]); break;
        case XOC_OP_PUSH:       snprintf(buf, len, "%s  PUSH       %s = %s"         , label, opr[0], opr[1]); break;
        case XOC_OP_UNARY:      snprintf(buf, len, "%s  UNARY      %s = %s %s"      , label, opr[1], opr[0], opr[2]); break;
        case XOC_OP_BINARY:     snprintf(buf, len, "%s  BINARY     %s = %s %s %s"   , label, opr[1], opr[2], opr[0], opr[3]); break;
        case XOC_OP_ASSIGN:     snprintf(buf, len, "%s  ASSIGN     %s = %s"         , label, opr[0], opr[1]); break;
        case XOC_OP_JMP:        snprintf(buf, len, "%s  JMP        %s"              , label, opr[0]); break;
        case XOC_OP_JMP_IF:     snprintf(buf, len, "%s  JMP_IF     %s, %s"          , label, opr[0], opr[1]); break;
        case XOC_OP_JMP_IFN:    snprintf(buf, len, "%s  JMP_IFN    %s, %s"          , label, opr[0], opr[1]); break;
        case XOC_OP_JMP_IFEQ:   snprintf(buf, len, "%s  JMP_IFEQ   %s, %s == %s"    , label, opr[0], opr[1], opr[2]); break;
        case XOC_OP_JMP_IFNE:   snprintf(buf, len, "%s  JMP_IFNE   %s, %s != %s"    , label, opr[0], opr[1], opr[2]); break;
        case XOC_OP_RET:        snprintf(buf, len, "%s  RET        "                , label); break;
        default:                snprintf(buf, len, "%s  UNKNOW     %d"              , label, inst->opc); break;
    }
}