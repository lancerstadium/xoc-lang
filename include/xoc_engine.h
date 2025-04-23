/**
 * @file    xoc_engine.h
 * @brief   XOC Engine Declarations
 * @author  lancerstadium
 * @details This file contains the declarations for XOC Engine.
 * @note    Use standard bare C headers for cross-compatibility.
 * 
 *          Virtual Machine Stack Layout for 64-bit Args:
 * 
 *              +---------------------------+
 *              |   ...                     |               <-- Address 0
 *              |   ...                     |
 *              |   ...                     |               <-- Stack Origin
 *              |   ...                     |
 *              |   ...                     |               <-- Stack Top
 *              |   Local                   |
 *              |   Local                   |
 *              |   ...                     |
 *              |   Temp                    |
 *              |   Temp                    |
 *              |   ...                     | 
 *              |   ...                     |      
 *              |   ...                     |               <-- Address 0
 *              +---------------------------+
 * 
 */

#ifndef XOC_Engine_H
#define XOC_Engine_H

#include "xoc_lexer.h"
#include "xoc_types.h"

typedef enum xoc_engine_status{
    ENGINE_STATUS_OK,
    ENGINE_STATUS_ERROR
} engine_status_t;



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
    fiber_t* src;
    heappage_t* head;
    heappage_t* tail;
};

struct xoc_chunkheader {
    bool is_stack;
    int64_t ref_cnt;
    int64_t size;
    int64_t pc;                     /* Optional: instruction pointer when allocated */
    extfn_t free;                   /* Optional: callback when ref_cnt reaches 0 */
};

struct xoc_fiber {
    bool is_alive;
    arg_t   * stk;
    arg_t   * stk_top;
    arg_t   * stk_base;
    int       stk_size;
    int64_t   pc;                   /* Execute Instruction pointer */
    inst_t  * code;                 /* Instructions */
    fiber_t * src;
    engine_t* eng;
};

struct xoc_engine {
    
    heap_t   heap;
    fiber_t* fibs;
    fiber_t* fib_cur;
    log_t  * log;
};


void engine_init    (engine_t* eng, int stack_size, bool is_filesys_enabled, log_t* log);
void engine_free    (engine_t* eng);
void engine_reset   (engine_t* eng);
void engine_loop    (engine_t* eng);




#endif /* XOC_Engine_H */