#ifndef js_init_ffi_h_
#define js_init_ffi_h_

#include "js/ffi/inc/ffi.h"

#ifdef __cplusplus
extern "C" {
#endif

JSModuleDef *js_init_module_ffi(JSContext *ctx, const char *module_name);

#ifdef __cplusplus
}
#endif

#endif