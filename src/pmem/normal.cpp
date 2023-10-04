
// #include "log.h"
#include "pmem/inc/pmem.h"
#include <loguru.hpp>

#include <mimalloc.h>
#include <sys/stat.h>

void pmem_free_alloc(pmem *ptr) {
#ifdef PMEM_DEBUG
    LOG_F(INFO, "pmem_free_alloc");
#endif
    mi_free(ptr);
}

static void *_malloc(size_t size, void *_ptr) { return mi_malloc(size); }

static void *_realloc(void *ptr, size_t newsize, void *_ptr) {
    return mi_realloc(ptr, newsize);
}

static size_t _usable_size(const void *ptr, void *_ptr) {
    return mi_usable_size(ptr);
}

static void _free(void *ptr, void *_ptr) { return mi_free(ptr); }

pmem *pmem_new_alloc(size_t size, pmem_t type, void *_ptr) {
    pmem *r = (pmem *)mi_malloc(sizeof(pmem));
    if (type == normal) {
#ifdef PMEM_DEBUG
        LOG_F(INFO, "pmem_new_alloc: normal");
#endif

        r->type = normal;
        r->alloc = _malloc;
        r->realloc = _realloc;
        r->usable_size = _usable_size;
        r->free = _free;
    } else {
        LOG_F(ERROR, "not support this type:{%d}", type);
        mi_free(r);
        return NULL;
    }

    return r;
}