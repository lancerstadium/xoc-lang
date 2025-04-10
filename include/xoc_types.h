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

struct xoc_sign {
    bool is_method;
    int num_param;
    int num_default_param;
    int offset;
    param_t* param[XOC_MAX_PAR_SIZE];
    type_t* type;
};

struct xoc_type {
    typekind_t kind;
    bool is_expr_list;
    bool is_enum;
    int blk;
    int num_item;
    type_t* base;
    type_t* next;
    ident_t* ident;                 /** For types that have identifiers */
    union {
        field_t** field;
        econst_t** econst;
        sign_t** sign;
    };
};


struct xoc_types {
    bool is_enabled;
    type_t* head;
    type_t* tail;
    errp_t* errp;
};
 
#endif /* XOC_TYPES_H */