#include <xoc_engine.h>

#include <stdlib.h>
#include <stdio.h>

void heap_init(heap_t* heap) {
    heap->head = heap->tail = NULL;
    heap->size = 0;
    heap->pid = 1;
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
    int chk_size = xoc_align(sizeof(chunkheader_t) + xoc_align(size + 1, sizeof(int64_t)), XOC_MIN_MEM_CHUNK);
    
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


void fiber_init(fiber_t* fib, engine_t* eng, int stack_size) {
    fib->src = NULL;
    fib->eng = eng;
    fib->stk = heap_alc_chk(&eng->heap, stack_size * sizeof(uint64_t), true);
    fib->stk_top = fib->stk_base = fib->stk + stack_size * sizeof(uint64_t);
    fib->stk_size = stack_size * sizeof(uint64_t);
    fib->is_alive = true;
}

void fiber_unwind_stk(fiber_t* fib) {

}


void fiber_change_stk_ref_cnt(fiber_t* fib, char* ptr, int delta) {
    if(ptr >= fib->stk_top && ptr < (fib)) {
    }
}


void fiber_info(fiber_t* fib, char* buf, int len) {
    snprintf(buf, len, "fiber %p stack %p size %ld/%d is_alive %d", fib, fib->stk, fib->stk_top - fib->stk_base, fib->stk_size, fib->is_alive);
}


void engine_init(engine_t* eng, int stack_size, bool is_filesys_enabled, log_t* log) {
    heap_init(&eng->heap);
    eng->fibs = (fiber_t*)malloc(sizeof(fiber_t));
    fiber_init(eng->fibs, eng, stack_size);
    eng->fib_cur = eng->fibs;
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