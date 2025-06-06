#include <xoc_common.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* xoc_strdup(const char* src) {
    if (!src) {
        return NULL;
    }
    return strdup(src);
}

uint64_t xoc_hash(const char* str) {
    uint64_t hash = 5381;
    char ch;
    while ((ch = *str++)) {
        hash = ((hash << 5) + hash) + ch;
    }
    return hash;
}

int64_t xoc_align(int64_t size, int64_t align) {
    return ((size + align - 1) / align) * align;
}


double xoc_pow(double base, int exp) {
    double result = 1.0;
    while (exp > 0) {
        if (exp & 1) {
            result *= base;
        }
        base *= base;
        exp >>= 1;
    }
    return result;
}

bool xoc_isident(char ch) {
    return (ch >= 'a' && ch <= 'z') || 
           (ch >= 'A' && ch <= 'Z') || 
           (ch >= '0' && ch <= '9') || 
           (ch == '_');
}

int xoc_ch2digit(char ch, int base) {
    switch (base) {
        case 2  : return ch >= '0' && ch <= '1' ? ch - '0' : -1;
        case 8  : return ch >= '0' && ch <= '7' ? ch - '0' : -1;
        case 10 : return ch >= '0' && ch <= '9' ? ch - '0' : -1;
        case 16 : return ch >= '0' && ch <= '9' ? ch - '0' : 
                         ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 : 
                         ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : -1;
    }
    return -1;
}

void info_init(info_t* info, const char* file, const char* func, int row, int pos, int code) {
    info->file = xoc_strdup(file);
    info->func = xoc_strdup(func);
    info->row = row;
    info->pos = pos;
    info->code = code;
    info->msg = NULL;
}

void info_setpos(info_t* info, const char* file, const char* func, int row, int pos) {
    info_free(info);
    info->file = xoc_strdup(file);
    info->func = xoc_strdup(func);
    info->row = row;
    info->pos = pos;
}

void info_setmsg(info_t* info, const char* fmt, va_list args) {
    if (info->msg) {
        free(info->msg);
        info->msg = NULL;
    }
    va_list argscp;
    va_copy(argscp, args);
    const int msg_len = vsnprintf(NULL, 0, fmt, args);
    if (msg_len > 0) {
        info->msg = malloc(msg_len + 1);
        vsnprintf(info->msg, msg_len + 1, fmt, argscp);
    }
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

static void log_fn_default(void* context, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void log_fn_info(void* context, const char* fmt, ...) {

    info_t* info = (info_t*)context;
    va_list args;
    va_start(args, fmt);
    if (!context) {
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        return;
    }
    va_list argscp;
    va_copy(argscp, args);
    int msg_len = vsnprintf(NULL, 0, fmt, args);
    if (msg_len > 0) {
        if(info->msg) {
            free(info->msg);
            info->msg = NULL;
        }
        info->msg = malloc(msg_len + 1);
        vsnprintf(info->msg, msg_len + 1, fmt, argscp);
        printf("\n%s:%d:%d: %s", info->file, info->row, info->pos, info->msg);
    }
    va_end(args);
}


void log_init(log_t* log, void* context, log_fn_t fn) {
    log->context = context;
    log->fmt = fn ? fn : log_fn_default;
}


void bucket_set(bucket_t* bucket, uint64_t key, char* ptr, int size) {
    bucket->key = key;
    if (size > 0) {
        if (bucket->data) {
            free(bucket->data);
        }
        bucket->data = malloc(size);
        memcpy(bucket->data, ptr, size);
    }
}

void map_init(map_t* map) {
    map->size = 0;
    memset(map->buk, 0, sizeof(bucket_t*) * XOC_MAX_HASH_SIZE);
}

bucket_t* map_find(map_t* map, uint64_t key) {
    bucket_t* p = map->buk[key % XOC_MAX_HASH_SIZE];
    while (p) {
        if (p->key == key) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

uint64_t map_add(map_t* map, char* ptr, int size) {
    uint64_t key = xoc_hash(ptr);
    bucket_t* p = map_find(map, key);
    if (!p) {
        int index = key % XOC_MAX_HASH_SIZE;
        bucket_t* new_bucket = (bucket_t*)malloc(sizeof(bucket_t));
        memset(new_bucket, 0, sizeof(bucket_t));
        bucket_set(new_bucket, key, ptr, size);
        p = map->buk[index];
        if (!p) {
            map->buk[index] = new_bucket;
        } else {
            while(p) {
                if (!p->next) {
                    p->next = new_bucket;
                    break;
                }
                p = p->next;
            }
        }
        map->size++;
    }
    return key;
}

char* map_get(map_t* map, uint64_t key) {
    bucket_t* p = map_find(map, key);
    if (p) {
        return p->data;
    }
    return NULL;
}

void map_del(map_t* map, uint64_t key) {
    // find prev & now bucket
    bucket_t* prev = NULL;
    bucket_t* p = map->buk[key % XOC_MAX_HASH_SIZE];
    while (p) {
        if (p->key == key) {
            break;
        }
        prev = p;
        p = p->next;
    }

    // delete
    if (p) {
        if (prev) {
            prev->next = p->next;
        } else {
            map->buk[key % XOC_MAX_HASH_SIZE] = NULL;
        }
        free(p->data);
        free(p);
        map->size--;
    }
}

void map_free(map_t* map) {
    for (int i = 0; i < XOC_MAX_HASH_SIZE; i++) {
        bucket_t* p = map->buk[i];
        while (p) {
            bucket_t* next = p->next;
            free(p->data);
            free(p);
            p = next;
        }
    }
}


void pool_init(pool_t* pool) {
    pool->head = pool->tail = NULL;
}

void pool_free(pool_t* pool) {
    blob_t *p = pool->head;
    while (p) {
        blob_t *next = p->next;
        free(p->data);
        free(p);
        p = next;
    }
}

char* pool_alc(pool_t* pool, int size) {
    blob_t *p = (blob_t*)malloc(sizeof(blob_t));
    p->data = (char*)malloc(size * sizeof(char));
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

int pool_cap(pool_t* pool) {
    blob_t *p = pool->head;
    int cap = 0;
    while (p) {
        p = p->next;
        cap++;
    }
    return cap;
}

char* pool_at(pool_t* pool, int idx) {
    blob_t *p = pool->head;
    while (p && idx--) {
        p = p->next;
    }
    return p ? p->data : NULL;
}

blob_t* pool_nin(pool_t* pool, const char* ptr) {
    blob_t *p = pool->head;
    while (p) {
        if(p->data + sizeof(uint32_t) * 3 == ptr) {
            break;
        }
        p = p->next;
    }
    return p;
}

char* pool_nat(pool_t* pool, int idx) {
    blob_t *p = pool->head;
    while (p && idx--) {
        p = p->next;
    }
    return p ? p->data + sizeof(uint32_t) * 3 : NULL;
}

blob_t* pool_nget(pool_t* pool, const char* ptr, uint32_t* align, uint32_t* size, uint32_t* capacity) {
    blob_t *p = pool_nin(pool, ptr);
    if (p) {
        *align = *(uint32_t*)p->data;
        *size = *(uint32_t*)(p->data + sizeof(uint32_t));
        *capacity = *(uint32_t*)(p->data + sizeof(uint32_t) * 2);
    } else {
        *align = 0;
        *size = 0;
        *capacity = 0;
    }
    return p;
}

blob_t* pool_nset(pool_t* pool, const char* ptr, uint32_t align, uint32_t size, uint32_t capacity) {
    blob_t *p = pool_nin(pool, ptr);
    if (p) {
        *(uint32_t*)p->data = align;
        *(uint32_t*)(p->data + sizeof(uint32_t)) = size;
        *(uint32_t*)(p->data + sizeof(uint32_t) * 2) = capacity;
    }
    return p;
}
    

char* pool_nalc(pool_t *pool, int align, int size) {
    char* data = pool_alc(pool, sizeof(uint32_t) * 3 + align * size);
    *(uint32_t*)data = align;                                   // Align (Byte)
    *(uint32_t*)(data + sizeof(uint32_t)) = 0;                  // Size (Number)
    *(uint32_t*)(data + sizeof(uint32_t) * 2) = size;           // Capacity (Number)
    return data + sizeof(uint32_t) * 3;
}

char* pool_nrlc(pool_t* pool, char** pptr, int size) {
    // Get align, size, capacity
    uint32_t align, org_size, org_capacity;
    // Find blob containing ptr
    blob_t *p = pool_nget(pool, *pptr, &align, &org_size, &org_capacity);
    if (!p) {
        return NULL;
    }
    // Deal with realloc
    if (org_size < size && org_capacity < size) {
        // realloc blob & update size & capacity & memcpy
        char* new_data = (char*)malloc(sizeof(uint32_t) * 3 + align * size);
        memcpy(new_data + sizeof(uint32_t) * 3, p->data + sizeof(uint32_t) * 3, align * org_size);
        pool_nset(pool, new_data, align, org_size, size);
        free(p->data);
        p->data = new_data;
        *pptr = new_data + sizeof(uint32_t) * 3;
        return *pptr;
    } else if (org_size < size && org_capacity >= size) {
        // update size
        pool_nset(pool, *pptr, align, org_size, org_capacity);
        return *pptr;
    } else {
        return *pptr;
    }
}

char* pool_npush(pool_t* pool, char** pptr, char* new, int size) {
    // Get align, size, capacity
    uint32_t align, org_size, org_capacity;
    // Find blob containing ptr
    blob_t *p = pool_nget(pool, *pptr, &align, &org_size, &org_capacity);
    if (!p) {
        return NULL;
    }
    // Deal with push
    if (org_size + size <= org_capacity) {
        // memcpy & update size
        memcpy(p->data + sizeof(uint32_t) * 3 + align * org_size, new, align * size);
        pool_nset(pool, *pptr, align, org_size + size, org_capacity);
        return p->data + sizeof(uint32_t) * 3 + align * org_size;
    } else {
        // expand double capacity is not enough, calculate new capacity
        int new_capacity = org_capacity;
        while (new_capacity < org_size + size) {
            new_capacity *= 2;
        }
        char* new_ptr = pool_nrlc(pool, pptr, new_capacity);
        memcpy(new_ptr + align * org_size, new, align * size);
        pool_nset(pool, new_ptr, align, org_size + size, new_capacity);
        return new_ptr + align * org_size;
    }
}


char* pool_npop(pool_t* pool, const char* ptr, int size) {
    // Get align, size, capacity
    uint32_t align, org_size, org_capacity;
    // Find blob containing ptr
    blob_t *p = pool_nget(pool, ptr, &align, &org_size, &org_capacity);
    if (!p) {
        return NULL;
    }
    // Deal with pop
    if (org_size >= size) {
        // update size
        pool_nset(pool, ptr, align, org_size - size, org_capacity);
        return p->data + sizeof(uint32_t) * 3 + align * (org_size - size);
    } else {
        return NULL;
    }
}

