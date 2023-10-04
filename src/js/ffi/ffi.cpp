

#include "js/jsc/inc/jsc.h"
#include "js_init.h"
#include "load_dynamic.h"

#include <stdio.h>

#include "pmem/inc/pmem.h"
#include <loguru.hpp>

std::atomic<cmodule_list_t *> cmodule_list{nullptr};
#define cl_load cmodule_list.load(std::memory_order_acquire)
#define MAX_NAME 128

void cmodule_list_free() {

    if (cl_load == nullptr)
        return;
    while (cl_load->len > 0) {
        cmodule_t *t = &cl_load->array[--cl_load->len];
        mi_free(t->name);
    }
    mi_free(cl_load->array);
    cl_load->array = nullptr;
    cl_load->cap = 0;
}

void cmodule_list_add(const char *name, ffi_cmodule_t fn) {

    if (!cl_load) {
        cmodule_list.store((cmodule_list_t *)mi_malloc(sizeof(cmodule_list_t)),
                           std::memory_order_release);
        cl_load->array = nullptr;
        cl_load->cap = 0;
        cl_load->len = 0;
    }

    if (cl_load->len >= cl_load->cap) {
        size_t newcap = cl_load->cap + (cl_load->cap >> 1) + 4;
        cmodule_t *a = (cmodule_t *)mi_realloc(
            cl_load->array, sizeof(cl_load->array[0]) * newcap);

        if (!a) {
            // log_error("cmodule_list_add: realloc error", 0);
            LOG_F(ERROR, "realloc error");
            return;
        }

        cl_load->array = a;
        cl_load->cap = newcap;
    }

    cl_load->array[cl_load->len].name = mi_strdup(name);
    cl_load->array[cl_load->len].fn = fn;
    ++cl_load->len;
}

void panda_js_init_ffi() {
#ifdef JS_DEBUG
    LOG_F(INFO, "panda_js_init_ffi");
#endif
    cmodule_list_add("ffi", js_init_module_ffi);
}

void panda_js_free_ffi() {
#ifdef JS_DEBUG
    LOG_F(INFO, "panda_js_free_ffi");
#endif
    cmodule_list_free();
    mi_free((void *)cl_load);
    lib_list_free();
}

static ffi_cmodule_t ffi_call_init_cmodule(const char *cmodule_name) {
    return load_dynamic(cmodule_name, "js_init_module");
}

JSModuleDef *panda_js_init_cmodule(JSContext *ctx, const char *cmodule_name) {
#ifdef JS_DEBUG
    LOG_F(INFO, "panda_js_init_cmodule:{%s}", cmodule_name);
#endif

    if (!cmodule_name) {
        LOG_F(ERROR, "cmodule_name is null");
        return nullptr;
    }

    if (!strcmp(cmodule_name, "std")) {
        LOG_F(WARNING, "js_init_module_std be call!");
        return js_init_module_std(ctx, cmodule_name);
    }
    if (!strcmp(cmodule_name, "os")) {
        LOG_F(WARNING, "js_init_module_os be call!");
        return js_init_module_os(ctx, cmodule_name);
    }

    if (has_suffix(cmodule_name, p_suffix)) {
        char module_name[MAX_NAME];
        int module_len = strlen(cmodule_name) - strlen(p_suffix);

        if (module_len > 127) {
            LOG_F(ERROR, "module_name too len (< 128)\nname:%s", cmodule_name);
            return nullptr;
        }
        int pos = 0;
        for (int i = 0; i < module_len; ++i) {
            if (cmodule_name[i] == '\\' or cmodule_name[i] == '/')
                pos = i;
        }
        snprintf(module_name, MAX_NAME - 1, "%.*s", module_len - pos - 1,
                 cmodule_name + pos + 1);

        ffi_cmodule_t fn = ffi_call_init_cmodule(cmodule_name);

        if (fn == nullptr)
            return nullptr;

        return fn(ctx, module_name);
    }

    if (!cl_load) {
        LOG_F(ERROR, "cmodule_list is not initialized!");
        return nullptr;
    }

    int size = cl_load->len;

    for (int i = 0; i < size; ++i) {
        if (!strcmp(cmodule_name, cl_load->array[i].name)) {
            return cl_load->array[i].fn(ctx, cmodule_name);
        }
    }

    LOG_F(ERROR, "no found cmodule:{%s}", cmodule_name);
    return nullptr;
}
