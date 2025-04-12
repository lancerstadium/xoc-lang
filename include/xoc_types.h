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

struct xoc_param {
    identname_t name;
    unsigned int key;
    const_t val;
    type_t* type;
};

struct xoc_func {
    bool is_method;
    int num_param;
    int num_default_param;
    int offset;
    param_t* param[XOC_MAX_PAR_SIZE];
    type_t* type;
};

// struct xoc_type {
//     typekind_t kind;
//     const_t val;
//     // bool is_expr_list;
//     // bool is_enum;
//     // int blk;
//     // int num_item;
//     // type_t* base;
//     // type_t* next;
//     // ident_t* ident;                 /** For types that have identifiers */
//     // union {
//     //     field_t** field;
//     //     econst_t** econst;
//     //     func_t** func;
//     // };
// };

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

 
#endif /* XOC_TYPES_H */