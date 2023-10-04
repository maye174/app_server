
#ifndef jsc_h_
#define jsc_h_

#ifdef _MSC_VER
#ifndef pjs_dll
#define pjs_dll __declspec(dllimport)
#endif
#else
#ifndef pjs_dll
#define pjs_dll
#endif // !pjs_dll

#endif // _MSC_VER

#include "pmem/inc/pmem.h"
// typedef struct pmem pmem;

#ifdef __cplusplus
extern "C" {
#endif

#include "tp/inc/quickjspp/cutils.h"
#include "tp/inc/quickjspp/quickjs-libc.h"

JSRuntime *panda_jsc_new_rt(pmem *alloc);
void panda_jsc_free_rt(JSRuntime *p);
pjs_dll JSModuleDef *js_init_module(JSContext *ctx, const char *module_name);

typedef struct namelist_t {
    char **name_array;
    int len;
    int cap;
} namelist_t;

void namelist_add(namelist_t *lp, const char *name);
void namelist_free(namelist_t *lp);
char *namelist_find(namelist_t *lp, const char *name);
void namelist_add_cmodule(namelist_t *n);

// js bytecode
typedef struct panda_js_bc {
    BOOL byte_swap;
    uint32_t bytecode_len;
    uint8_t *bytecode;
    namelist_t *cmodule_list;
    struct panda_js_bc *next;
} panda_js_bc;

panda_js_bc *panda_new_js_bc(JSContext *ctx);
void panda_free_js_bc(JSContext *ctx, panda_js_bc *ptr);

panda_js_bc *panda_js_toBytecode(JSRuntime *rt, JSContext *ctx,
                                 const char *filename);

typedef struct panda_js_obj {
    BOOL byte_swap;
    int _padding;
    JSValue obj;
    namelist_t *cmodule_list;
    struct panda_js_obj *next;
} panda_js_obj;

panda_js_obj *panda_new_js_obj(JSContext *ctx);
void panda_free_js_obj(JSContext *ctx, panda_js_obj *ptr);

panda_js_obj *panda_js_toObj(JSRuntime *rt, JSContext *ctx,
                             const char *filename);

typedef enum {
    bytecode,
    obj,
} panda_js_t;

typedef struct panda_js {
    panda_js_t type;
    int _padding;
    void *ptr;
    JSContext *ctx;
} panda_js;

panda_js *panda_new_js();
void panda_free_js(panda_js *pjs);

panda_js *panda_js_to(JSRuntime *rt, const char *filename, panda_js_t type);

void panda_js_run(panda_js *pjs);

#ifdef __cplusplus
}
#endif

#endif // !jsc_h_