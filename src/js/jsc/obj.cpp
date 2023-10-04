
#include "js/ffi/inc/ffi.h"
#include "js/jsc/inc/jsc.h"

#include <cstring>

#include <loguru.hpp>

static JSModuleDef *jsc_module_loader(JSContext *ctx, const char *module_name,
                                      void *opaque) {
#ifdef JS_DEBUG
    LOG_F(INFO, "loader modulename:{%s}", module_name);
#endif
    JSModuleDef *m;
    char *find_buf;
    panda_js_obj *jsc_o = (panda_js_obj *)opaque;

    /* check if it is a declared C or system module */
    find_buf = namelist_find(jsc_o->cmodule_list, module_name);
    if (find_buf) {

        m = panda_js_init_cmodule(ctx, find_buf);
        if (!m)
            LOG_F(ERROR, "can't init module: %s", find_buf);

    } else if (has_suffix(module_name, p_suffix)) {

        m = panda_js_init_cmodule(ctx, module_name);
        if (!m)
            LOG_F(ERROR, "can't init module: %s", module_name);

    } else {

        size_t buf_len;
        uint8_t *buf;
        JSValue func_val;

        if (has_suffix(module_name, ".js")) {
            buf = js_load_file(ctx, &buf_len, module_name);
        } else {
            size_t len = strlen(module_name);
            char *module_name_buf = (char *)js_malloc(ctx, len + 4);
            snprintf(module_name_buf, len + 4, "%s.js", module_name);
            buf = js_load_file(ctx, &buf_len, module_name_buf);
            js_free(ctx, module_name_buf);
        }
        if (!buf) {
            LOG_F(ERROR, "could not load module filename '%s'", module_name);
            return nullptr;
        }

        /* compile the module */
        func_val = JS_Eval(ctx, (char *)buf, buf_len, module_name,
                           JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
        js_free(ctx, buf);

        if (JS_IsException(func_val)) {
            LOG_F(ERROR, "eval error");
            js_std_dump_error(ctx);
            return nullptr;
        }

        while (jsc_o->next != nullptr) {
            jsc_o = jsc_o->next;
        }
        jsc_o->next = panda_new_js_obj(ctx);
        jsc_o->obj = func_val;

        /* the module is already referenced, so we must free it */
        m = (JSModuleDef *)JS_VALUE_GET_PTR(func_val);
    }
    return m;
}

static void compile_file(JSContext *ctx, panda_js_obj *jsc_o,
                         const char *filename) {
#ifdef JS_DEBUG
    LOG_F(INFO, "compile_file: %s", filename);
#endif
    uint8_t *buf;
    int eval_flags;
    JSValue obj;
    size_t buf_len;

    buf = js_load_file(ctx, &buf_len, filename);
    if (!buf) {
        LOG_F(ERROR, "Could not load: %s", filename);
        return;
    }
    eval_flags = JS_EVAL_FLAG_COMPILE_ONLY;
    int module = JS_DetectModule((const char *)buf, buf_len);

    if (module)
        eval_flags |= JS_EVAL_TYPE_MODULE;
    else
        eval_flags |= JS_EVAL_TYPE_GLOBAL;

    obj = JS_Eval(ctx, (char *)buf, buf_len, filename, eval_flags);
    js_free(ctx, buf);

    if (JS_IsException(obj)) {
        LOG_F(ERROR, "eval error");
        js_std_dump_error(ctx);
        return;
    }

    jsc_o->obj = obj;
}

panda_js_obj *panda_new_js_obj(JSContext *ctx) {
#ifdef JS_DEBUG
    LOG_F(INFO, "memory apply");
#endif
    panda_js_obj *r = (panda_js_obj *)js_malloc(ctx, sizeof(panda_js_obj));
    if (!r) {
        LOG_F(ERROR, "Cannot apply for memory");
        return nullptr;
    }
    r->byte_swap = FALSE;
    r->cmodule_list = (namelist_t *)js_malloc(ctx, sizeof(namelist_t));
    r->cmodule_list->name_array = nullptr;
    r->cmodule_list->len = 0;
    r->cmodule_list->cap = 0;
    r->next = nullptr;
    return r;
}

void panda_free_js_obj(JSContext *ctx, panda_js_obj *ptr) {
    if (ptr == nullptr)
        return;
#ifdef JS_DEBUG
    LOG_F(INFO, "memory free");
#endif
    namelist_free(ptr->cmodule_list);
    js_free(ctx, ptr->cmodule_list);
    panda_free_js_obj(ctx, ptr->next);
    js_free(ctx, ptr);
}

panda_js_obj *panda_js_toObj(JSRuntime *rt, JSContext *ctx,
                             const char *filename) {
#ifdef JS_DEBUG
    LOG_F(INFO, "panda_js_toObj filename: {%s}", filename);
#endif
    panda_js_obj *jsc_o = panda_new_js_obj(ctx);

    if (!jsc_o) {
        return nullptr;
    }

    namelist_add_cmodule(jsc_o->cmodule_list);
    JS_SetModuleLoaderFunc(rt, nullptr, jsc_module_loader, jsc_o);
    compile_file(ctx, jsc_o, filename);

    return jsc_o;
}
