
#include "js_init.h"
#include "js/jsc/inc/jsc.h"

// static const char *suffix = p_suffix;

static const JSCFunctionListEntry js_ffi_funcs[] = {
    // JS_CFUNC_DEF("fib", 1, js_fib),
    JS_PROP_STRING_DEF("p_suffix", p_suffix, JS_PROP_ENUMERABLE),
};

static int js_ffi_init(JSContext *ctx, JSModuleDef *m) {
    return JS_SetModuleExportList(ctx, m, js_ffi_funcs, countof(js_ffi_funcs));
}

JSModuleDef *js_init_module_ffi(JSContext *ctx, const char *module_name) {
    JSModuleDef *m;
    m = JS_NewCModule(ctx, module_name, js_ffi_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_ffi_funcs, countof(js_ffi_funcs));
    return m;
}
