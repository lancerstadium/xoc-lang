/**
 * @file    xoc_ident.h
 * @brief   XOC Identifier Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC Identifier.
 */

#ifndef XOC_IDENT_H
#define XOC_IDENT_H
 
#include "xoc_common.h"

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


struct xoc_idents {
    ident_t* head;
    ident_t* tail;
    ident_t* last_temp_var;
    int temp_var_suffix;  
    info_t* info;
    errp_t* errp;
};



 #endif // XOC_IDENT_H