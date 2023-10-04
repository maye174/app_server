add_requires("mimalloc")

target("pmem")
    set_kind("static")
    add_files("./normal.cpp")
    add_packages("mimalloc", "loguru")

target("pmem_test")
    add_deps("pmem")
    set_kind("binary")
    add_files("./test.c")
