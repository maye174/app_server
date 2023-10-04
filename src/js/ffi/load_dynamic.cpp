
#include "load_dynamic.h"
#include "pmem/inc/pmem.h"

#include <loguru.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#define LIB_T HMODULE
#else
#include <dlfcn.h>
#define LIB_T void *
#endif

typedef struct {
    char *filename;
    LIB_T lib;
} lib_t;

typedef struct {
    lib_t *array;
    int cap;
    int len;
} lib_list_t;

static std::atomic<lib_list_t *> list{nullptr};
#define l_load list.load(std::memory_order_acquire)

void lib_list_free() {

    if (l_load == nullptr)
        return;
    while (l_load->len > 0) {
        lib_t *t = &l_load->array[--l_load->len];
        mi_free(t->filename);
#if defined(_WIN32) || defined(_WIN64)
        if (t->lib)
            FreeLibrary(t->lib);
#else
        if (t->lib)
            dlclose(t->lib);
#endif
    }
    mi_free(l_load->array);
    l_load->array = nullptr;
    l_load->cap = 0;

    mi_free(l_load); // this is an exception in panda_engine
}

static void lib_list_add(const char *filename, LIB_T lib) {

    if (!l_load) {
        list.store((lib_list_t *)mi_malloc(sizeof(lib_list_t)),
                   std::memory_order_release);
        l_load->array = nullptr;
        l_load->cap = 0;
        l_load->len = 0;
    }

    if (l_load->len >= l_load->cap) {
        size_t newcap = l_load->cap + (l_load->cap >> 1) + 4;
        lib_t *a = (lib_t *)mi_realloc(l_load->array,
                                       sizeof(l_load->array[0]) * newcap);

        if (!a) {
            // log_error("lib_list_add: realloc error", 0);
            LOG_F(ERROR, "realloc error");
            return;
        }

        l_load->array = a;
        l_load->cap = newcap;
    }

    l_load->array[l_load->len].filename = mi_strdup(filename);
    l_load->array[l_load->len].lib = lib;
    ++l_load->len;
}

LIB_T search_list(const char *filename) {
    if (!l_load)
        return nullptr;
    for (int i = 0; i < l_load->len; ++i) {
        if (l_load->array[i].filename == filename) {
            return l_load->array[i].lib;
        }
    }
    return nullptr;
}

ffi_cmodule_t load_dynamic(const char *filename, const char *fn_name) {
    // log_debug("load_dynamic file:{%s}, fn_name:{%s}", filename, fn_name);
    LOG_F(INFO, "load_dynamic file:{%s}, fn_name:{%s}", filename, fn_name);

    ffi_cmodule_t func = nullptr;
    LIB_T lib = search_list(filename);

#if defined(_WIN32) || defined(_WIN64)

    if (!lib)
        lib = LoadLibrary(filename);
    if (lib) {
        func = (ffi_cmodule_t)GetProcAddress(lib, fn_name);
        if (func == nullptr) {
            LOG_F(ERROR, "load dynamic init func error: {%s}", fn_name);
            LOG_F(ERROR, "GetProcAddress failed, error = %ld", GetLastError());
            return nullptr;
        }
    } else {
        LOG_F(ERROR, "load dynamic error");
        return nullptr;
    }

#else

    if (!lib)
        lib = dlopen(filename, RTLD_LAZY);
    if (lib) {
        func = (ffi_cmodule_t)dlsym(lib, funcname);
        if (func == nullptr) {
            LOG_F(ERROR, "load dynamic init func error");
            return nullptr;
        }
    } else {
        LOG_F(ERROR, "load dynamic error");
        return nullptr;
    }

#endif

    lib_list_add(filename, lib);

    return func;
}