
#include "js/ffi/inc/ffi.h"
#include "js/jsc/inc/jsc.h"
#include "pmem/inc/pmem.h"

#include <loguru.hpp>

void namelist_add(namelist_t *lp, const char *name) {

    if (lp->len >= lp->cap) {
        size_t newcap = lp->cap + (lp->cap >> 1) + 4;
        char **a = (char **)mi_realloc(lp->name_array,
                                       sizeof(lp->name_array[0]) * newcap);

        if (!a) {
            // log_error("namelist_add: realloc error", 0);
            LOG_F(ERROR, "realloc error");
            return;
        }

        lp->name_array = a;
        lp->cap = newcap;
    }

    lp->name_array[lp->len++] = mi_strdup(name);
}

void namelist_free(namelist_t *lp) {
    if (lp == nullptr)
        return;
    while (lp->len > 0) {
        mi_free(lp->name_array[--lp->len]);
    }
    mi_free(lp->name_array);
    lp->name_array = nullptr;
    lp->cap = 0;
}

char *namelist_find(namelist_t *lp, const char *name) {

    for (int i = 0; i < lp->len; ++i) {
        if (!strcmp(lp->name_array[i], name))
            return lp->name_array[i];
    }
    return nullptr;
}

void namelist_add_cmodule(namelist_t *n) {

    if (!cmodule_list.load(std::memory_order_acquire)) {
        LOG_F(
            WARNING,
            "Cmodule_list is not initialized! Will enable the default module");
        namelist_add(n, "std");
        namelist_add(n, "os");
        return;
    }

    int size = cmodule_list.load(std::memory_order_acquire)->len;

    for (int i = 0; i < size; ++i) {
        namelist_add(
            n, cmodule_list.load(std::memory_order_acquire)->array[i].name);
    }
}