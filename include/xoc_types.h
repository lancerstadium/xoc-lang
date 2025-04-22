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

struct xoc_field {
    identname_t name;
    unsigned int key;
    int offset;
    type_t* type;
};

struct xoc_econst {
    identname_t name;
    unsigned int key;
    const_t val;
};

struct xoc_type {
    typekind_t kind;
    union {
        int8_t      I8;
        uint8_t     U8;
        int16_t     I16;
        uint16_t    U16;
        int32_t     I32;
        uint32_t    U32;
        int64_t     I64;
        uint64_t    U64;
        void*       Ptr;
        uint64_t    WPtr;
        float       F32;
        double      F64;
    };
};

struct xoc_ident {
    identkind_t kind;
    identname_t name;
    bool is_export;
    bool is_global;
    bool is_used;
    bool is_temp;
    int blk;
    int mod;
    int func_offset;
    unsigned int key;
    ident_t *next;
    union {
        void* ptr;
        int64_t offset;
        int64_t mod_val;
        const_t constant;
    };
    info_t info;
};

struct xoc_func {
    bool is_method;
    int num_param;
    int num_default_param;
    int64_t offset;
    type_t param[XOC_MAX_PAR_SIZE];
};

 
#endif /* XOC_TYPES_H */