/**
 * @file    xoc_engine.h
 * @brief   XOC Engine Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC Engine.
 * @note    Use standard bare C headers for cross-compatibility.
 */

#ifndef XOC_Engine_H
#define XOC_Engine_H

#include "xoc_lexer.h"
#include "xoc_types.h"

struct xoc_heappage {
    int id;
    int ref_cnt;
    int num_chk, num_occp, num_free, chk_size;
    char* data;
    heappage_t* prev;
    heappage_t* next;
};


struct xoc_heap {
    int size;
    int pid;
    heappage_t* head;
    heappage_t* tail;
};

struct xoc_chunkheader {
    bool is_stack;
    int64_t ref_cnt;
    int64_t size;
};

struct xoc_fiber {
    bool is_alive;
    char    * stk;
    char    * stk_top;
    char    * stk_base;
    int       stk_size;
    fiber_t * src;
    engine_t* eng;
};

struct xoc_engine {
    heap_t   heap;
    fiber_t* fibs;
    fiber_t* fib_cur;
    log_t  * log;
};


void engine_init(engine_t* eng, int stack_size, bool is_filesys_enabled, log_t* log);
void engine_free(engine_t* eng);




#endif /* XOC_Engine_H */