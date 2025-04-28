/**
 * @file    xoc_types.h
 * @brief   XOC Types Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC types.
 * @note    Use standard bare C headers for cross-compatibility:
 *          stdint.h, stdbool.h, stddef.h
 */

#ifndef XOC_TYPES_H
#define XOC_TYPES_H

#include "xoc_common.h"

struct xoc_type {
    typekind_t kind;
    uint64_t key;
    arg_t val;
    type_t* base;
    type_t* next;
};

struct xoc_ident {
    uint64_t key;

    identkind_t kind;
    char* name;

    uint64_t mod;
    bool is_used;
    bool is_temp;
    bool is_export;
    bool is_global;

    union {
        int64_t offset;
        arg_t constant;
    };
};

struct xoc_param {
    int64_t num_arg;
    int64_t num_res;
    int64_t num_alg;
    int64_t arg[];
};

struct xoc_func {
    bool is_method;
    int num_param;
    int num_default_param;
    int64_t offset;
    type_t param[XOC_MAX_PAR_SIZE];
};

struct xoc_inst {
    uint64_t label;
    opcode_t     opc;
    type_t*      opr[4];
};

type_t* type_alc(typekind_t kind);
void type_free(type_t* type);
type_t* type_i8(int8_t i8);
type_t* type_u8(uint8_t u8);
type_t* type_i16(int16_t i16);
type_t* type_u16(uint16_t u16);
type_t* type_i32(int32_t i32);
type_t* type_u32(uint32_t u32);
type_t* type_i64(int64_t i64);
type_t* type_u64(uint64_t u64);
type_t* type_f32(float f32);
type_t* type_f64(double f64);
type_t* type_char(char c);
type_t* type_str(uint64_t key);
type_t* type_tok(tokenkind_t tk);
type_t* type_tmp(int id);
type_t* type_idt(uint64_t key);
type_t* type_blk(int id);
type_t* type_lbl(uint64_t key);
type_t* type_dvc(devicekind_t dvc);
int type_size(type_t* type);
void type_info(type_t* type, char* buf, int len, map_t* syms);
void inst_info(inst_t* inst, char* buf, int len, map_t* syms);
 
#endif /* XOC_TYPES_H */