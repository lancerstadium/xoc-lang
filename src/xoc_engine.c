#include <xoc_engine.h>

#include <stdlib.h>
#include <stdio.h>


void heap_init(heap_t* heap);
void heap_free(heap_t* heap);
heappage_t* heap_add(heap_t* heap, int num_chk, int chk_size);
void heap_del(heap_t* heap, heappage_t* page);
chunkheader_t* heappage_get_chunkheader(heappage_t* page, char* ptr);
heappage_t* heap_find(heap_t* heap, char* ptr); 
heappage_t* heap_find_foralc(heap_t* heap, int size);
heappage_t* heap_get(heap_t* heap, int id);
char* heap_alc_chk(heap_t* heap, int size, bool is_stack);
int heap_change_chk_ref_cnt(heap_t* heap, heappage_t* page, char* ptr, int delta);
void fiber_init(fiber_t* fib, engine_t* eng, int stack_size);
bool fiber_unwind_stk(fiber_t* fib, arg_t** base, int* ip);
void fiber_change_stk_ref_cnt(fiber_t* fib, char* ptr, int delta);

arg_t* param_get_free(char* ptr) {
    static char param_layout_buf[sizeof(param_t) + 2 * sizeof(int64_t)];
    static arg_t param_buf[4 + 1] = {0};
    param_t* param = (param_t*)&param_layout_buf;
    arg_t* arg = param_buf + 4;

    param->num_arg = 2;
    param->num_alg = 1;
    param->num_res = 0;
    param->arg[0] = 0;          // No upvalue
    param->arg[1] = 0;          // Pointer to data to free

    arg[-4].Ptr = param;
    arg[ 0].Ptr = ptr;
    return arg;
}


void heap_init(heap_t* heap) {
    heap->head = heap->tail = NULL;
    heap->size = 0;
    heap->pid = 1;
    heap->src = NULL;
}

void heap_free(heap_t* heap) {
    heappage_t* page = heap->head;
    heappage_t* next;
    while (page) {
        next = page->next;
        if(page->data) {
            for(int i = 0; i < page->num_occp && page->num_free > 0; i++) {
                chunkheader_t* chk = (chunkheader_t*)(page->data + i * page->chk_size);
                if(chk->ref_cnt == 0) {
                    continue;
                }
                // free
                page->num_free--;
            }
            free(page->data);
            page->data = NULL;
        }
        free(page);
        page = next;
    }
}

heappage_t* heap_add(heap_t* heap, int num_chk, int chk_size) {
    heappage_t* page = (heappage_t*)malloc(sizeof(heappage_t));
    page->id = heap->pid++;
    const int size = num_chk * chk_size;
    page->data = (char*)malloc(size);
    if(!page->data) {
        free(page);
        return NULL;
    }
    page->num_chk = num_chk;
    page->num_occp = 0;
    page->num_free = 0;
    page->chk_size = chk_size;
    page->ref_cnt = 0;
    page->prev = heap->tail;
    page->next = NULL;
    if(!heap->head) {
        heap->head = heap->tail = page;
    } else {
        heap->tail->next = page;
        heap->tail = page;
    }
    heap->size += size;
    return heap->tail;
}

void heap_del(heap_t* heap, heappage_t* page) {
    heap->size -= page->num_chk * page->chk_size;
    if(page == heap->head) {
        heap->head = page->next;
    }
    if(page == heap->tail) {
        heap->tail = page->prev;
    }
    if(page->prev) {
        page->prev->next = page->next;
    }
    if(page->next) {
        page->next->prev = page->prev;
    }
    free(page->data);
    free(page);
    page = NULL;
}

chunkheader_t* heappage_get_chunkheader(heappage_t* page, char* ptr) {
    const int chk_offset = (ptr - page->data) % page->chk_size;
    return (chunkheader_t*)(ptr - chk_offset);
}

heappage_t* heap_find(heap_t* heap, char* ptr) {
    for (heappage_t* page = heap->head; page; page = page->next) {
        if(ptr >= page->data && ptr < page->data + page->num_chk * page->chk_size) {
            chunkheader_t* chk = heappage_get_chunkheader(page, ptr);
            if(chk->ref_cnt > 0) {
                return page;
            }
            return NULL;
        }
    }
    return NULL;
}

heappage_t* heap_find_foralc(heap_t* heap, int size) {
    heappage_t* best_page = NULL;
    int best_size = 1 << 30;
    for (heappage_t* page = heap->head; page; page = page->next) {
        if(page->num_occp < page->num_chk && page->chk_size >= size && page->chk_size < best_size) {
            best_page = page;
            best_size = page->chk_size;
        }
    }
    return best_page;
}

heappage_t* heap_get(heap_t* heap, int id) {
    heappage_t* page = heap->head;
    while (page) {
        if(page->id == id) {
            return page;
        }
        page = page->next;
    }
    return NULL;
}

char* heap_alc_chk(heap_t* heap, int size, bool is_stack) {
    int chk_size = xoc_align(sizeof(chunkheader_t) + xoc_align(size + 1, sizeof(arg_t)), XOC_MIN_MEM_CHUNK);
    
    heappage_t* page = heap_find_foralc(heap, chk_size);
    if(!page) {
        int num_chk = XOC_MIN_MEM_PAGE / chk_size;
        if (num_chk < 1) {
            num_chk = 1;
        }
        page = heap_add(heap, num_chk, chk_size);
    }
    chunkheader_t* chk = (chunkheader_t*)(page->data + page->num_occp * chk_size);
    chk->ref_cnt = 1;
    chk->size = size;
    chk->is_stack = is_stack;
    page->num_occp++;
    page->ref_cnt++;
    return (char*)chk + sizeof(chunkheader_t);
}

int heap_change_chk_ref_cnt(heap_t* heap, heappage_t* page, char* ptr, int delta) {
    chunkheader_t* chk = heappage_get_chunkheader(page, ptr);
    if(chk->ref_cnt <= 0 || page->ref_cnt < chk->ref_cnt) {
        return 0;
    }
    if(chk->free && chk->ref_cnt == 1 && delta == -1) {
        chk->free(param_get_free(ptr), NULL);
        page->num_free--;
    }
    chk->ref_cnt += delta;
    page->ref_cnt += delta;

    fiber_change_stk_ref_cnt(heap->src, ptr, delta);

    if(page->ref_cnt == 0) {
        heap_del(heap, page);
        return 0;
    }
    return chk->ref_cnt;
}


void fiber_init(fiber_t* fib, engine_t* eng, int stack_size) {
    fib->src = NULL;
    fib->eng = eng;
    fib->stk = (arg_t*)heap_alc_chk(&eng->heap, stack_size * sizeof(arg_t), true);
    fib->stk_top = fib->stk_base = fib->stk + stack_size;
    fib->stk_size = stack_size;
    fib->is_alive = true;
}

bool fiber_unwind_stk(fiber_t* fib, arg_t** base, int* ip) {
    if(*base == fib->stk + fib->stk_size - 1) {
        return false;
    }
    int ret_offset = (*base + 1)->I64;
    if(ret_offset == XOC_RET_FROM_FIB || ret_offset == XOC_RET_FROM_ENG) {
        return false;
    }
    *base = (arg_t*)((*base)->Ptr);
    if(ip)
        *ip = ret_offset;
    return true;
}


void fiber_change_stk_ref_cnt(fiber_t* fib, char* ptr, int delta) {
    if(ptr >= (char*)fib->stk_top && ptr < (char*)(fib->stk + fib->stk_size)) {
        arg_t* base = (arg_t*)fib->stk_base;
        param_t* param = base[-1].Ptr;
        while(ptr > (char*)(base + 1 + param->num_arg)) {
            if(!fiber_unwind_stk(fib, &base, NULL)) {
                fib->eng->log->fmt(NULL, "Illegal stack pointer %p", ptr);
                return;
            }
            param = base[-2].Ptr;
        }
        int64_t* stk_ref_cnt = &(base[-1].I64);
        *stk_ref_cnt += delta;
    }
}


void fiber_info(fiber_t* fib, char* buf, int len) {
    snprintf(buf, len, "fiber %p stack %p size %ld/%d is_alive %d", fib, fib->stk, fib->stk_top - fib->stk_base, fib->stk_size, fib->is_alive);
}


void fiber_fn_push(fiber_t* fib) {
}


static sysfn_t fiber_fn_tbl[] = {
    [XOC_OP_PUSH] = fiber_fn_push,
};


void engine_init(engine_t* eng, int stack_size, bool is_filesys_enabled, log_t* log) {
    heap_init(&eng->heap);
    eng->fibs = (fiber_t*)malloc(sizeof(fiber_t));
    fiber_init(eng->fibs, eng, stack_size);
    eng->heap.src = eng->fib_cur = eng->fibs;
    eng->log = log;
    char buf[256];
    fiber_info(eng->fibs, buf, 256);
    eng->log->fmt(NULL, "%s", buf);
}

void engine_free(engine_t* eng) {
    heappage_t* page = heap_find(&eng->heap, (char*)eng->fib_cur->stk);
    if(page) {
        page->ref_cnt--;
        heap_free(&eng->heap);
    }
    free(eng->fibs);
}

void engine_reset(engine_t* eng) {
    eng->fib_cur = eng->fibs;
    eng->fib_cur->stk_top = eng->fib_cur->stk_base = eng->fib_cur->stk + eng->fib_cur->stk_size;
}

void engine_halt(engine_t* eng) {
    
}

void engine_loop(engine_t* eng) {
    fiber_t* fib = eng->fib_cur;
    heap_t* heap = &eng->heap;
    log_t*   log = eng->log;

    while(1) {
        if(fib->stk_top - fib->stk < XOC_MIN_MEM_STACK) {
            log->fmt(NULL, "Stack overflow %ld/%d", fib->stk_top - fib->stk, fib->stk_size);
            break;
        }
        switch(fib->code[fib->pc].opc) {
            default: {
                log->fmt(NULL, "Unknown op %d", fib->code[fib->pc].opc);
                break;
            }
        }
    }
}