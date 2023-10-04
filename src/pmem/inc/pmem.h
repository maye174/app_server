#ifndef panda_malloc_h
#define panda_malloc_h

#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
extern void *mi_malloc(size_t size);
extern void *mi_realloc(void *p, size_t newsize);
extern size_t mi_usable_size(const void *p);
extern void mi_free(void *p);
extern char *mi_strdup(const char *s);

typedef enum {
    normal,
    stack,
    pool,
} pmem_t;

typedef struct {
    pmem_t type;
    void *_ptr;
    void *(*alloc)(size_t size, void *_ptr);
    void *(*realloc)(void *ptr, size_t newsize, void *_ptr);
    size_t (*usable_size)(const void *ptr, void *_ptr);
    void (*free)(void *ptr, void *_ptr);
} pmem;

pmem *pmem_new_alloc(size_t size, pmem_t type, void *_ptr);
void pmem_free_alloc(pmem *ptr);

#ifdef __cplusplus
}
#endif

#endif // panda_malloc_h