/**
 * @file    xoc_compiler.h
 * @brief   XOC Compiler Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC Compiler.
 * @note    Use standard bare C headers for cross-compatibility.
 */

#ifndef XOC_COMPILER_H
#define XOC_COMPILER_H

#include "xoc_common.h"
#include "xoc_types.h"
#include "xoc_ident.h"


struct xoc_compiler {
    mempool_t   pool;
    mods_t      mods;
    blks_t      blks;
    types_t     types;
    idents_t    idents;
    info_t      info;
    errp_t      errp;

    int argc;
    char **argv;
};



#endif /* XOC_COMPILER_H */