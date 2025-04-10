#include <xoc_common.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void info_init(info_t* info, const char* file, const char* func, int row, int pos, int code, const char* fmt, va_list args) {
    info_free(info);
    info->file = malloc(strlen(file) + 1);
    strcpy(info->file, file);
    info->func = malloc(strlen(func) + 1);
    strcpy(info->func, func);
    info->row = row;
    info->pos = pos;
    info->code = code;
    va_list argscp;
    va_copy(argscp, args);
    const int msg_len = vsnprintf(NULL, 0, fmt, args);
    info->msg = malloc(msg_len + 1);
    vsnprintf(info->msg, msg_len + 1, fmt, argscp);
    va_end(argscp);
}

void info_free(info_t* info) {
    if (info->file) {
        free(info->file);
        info->file = NULL;
    }
    if (info->func) {
        free(info->func);
        info->func = NULL;
    }
    if (info->msg) {
        free(info->msg);
        info->msg = NULL;
    }
}

void mempool_init(mempool_t *pool) {
    pool->head = pool->tail = NULL;
}

void mempool_free(mempool_t *pool) {
    memblob_t *p = pool->head;
    while (p) {
        memblob_t *next = p->next;
        free(p->data);
        free(p);
        p = next;
    }
}

char* mempool_alc(mempool_t *pool, int size) {
    memblob_t *p = (memblob_t*)malloc(sizeof(memblob_t));
    p->data = (char*)malloc(size);
    memset(p->data, 0, size);
    p->next = NULL;
    if (!pool->head) {
        pool->head = pool->tail = p;
    } else {
        pool->tail->next = p;
        pool->tail = p;
    }
    return pool->tail->data;
}
char* mempool_alc_str(mempool_t *pool, int len) {
    strdim_t dim = {len, len+1};
    char* dim_str = mempool_alc(pool, sizeof(strdim_t) + dim.cap);
    *(strdim_t*)dim_str = dim;
    char* data = dim_str + sizeof(strdim_t);
    data[len] = '\0';
    return data;
}