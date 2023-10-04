

#include "js/ffi/inc/ffi.h"
#include "js/jsc/inc/jsc.h"
#include "pmem/inc/pmem.h"

int main(int argc, char **argv) {

    // set_log(all);
    panda_js_init_ffi();
    // printf("hello\n");
    pmem *pma = pmem_new_alloc(0, normal, NULL);
    // printf("hello\n");
    JSRuntime *rt = panda_jsc_new_rt(pma);
    // printf("hello\n");

    panda_js *pjs = panda_js_to(rt, "./hello.js", obj);
    panda_js_run(pjs);
    panda_free_js(pjs);

    panda_jsc_free_rt(rt);
    pmem_free_alloc(pma);
    panda_js_free_ffi();

    return 0;
}
