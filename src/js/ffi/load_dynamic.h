#ifndef load_dynamic_h_
#define load_dynamic_h_

#include "js/ffi/inc/ffi.h"

#ifdef __cplusplus
extern "C" {
#endif

void lib_list_free();

ffi_cmodule_t load_dynamic(const char *filename, const char *fn_name);

#ifdef __cplusplus
}
#endif

#endif