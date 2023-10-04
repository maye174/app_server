#ifndef ffi_h_
#define ffi_h_

#ifdef __cplusplus
#include <atomic>
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
#define p_suffix ".dll"
#elif defined(__linux__)
#define p_suffix ".so"
#elif defined(__APPLE__)
#define p_suffix ".dylib"
#else
#error "Unknown_platform"
#endif

#include "tp/inc/quickjspp/quickjs.h"

typedef JSModuleDef *(*ffi_cmodule_t)(JSContext *ctx, const char *module_name);

typedef struct {
    char *name;
    ffi_cmodule_t fn;
} cmodule_t;

typedef struct {
    cmodule_t *array;
    int cap;
    int len;
} cmodule_list_t;

extern void cmodule_list_add(const char *name, ffi_cmodule_t fn);

extern void cmodule_list_free();

extern void panda_js_init_ffi();

extern void panda_js_free_ffi();

extern JSModuleDef *panda_js_init_cmodule(JSContext *ctx,
                                          const char *cmodule_name);

#ifdef __cplusplus
extern std::atomic<cmodule_list_t *> cmodule_list;
}
#endif

#endif