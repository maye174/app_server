add_requires("mimalloc")


option("pmem_debug")
    set_default(false)
    set_showmenu(true)
    set_category("option")
    set_description("Enable debug mode for pmem")
    add_defines("PMEM_DEBUG")

target("pmem")
    set_kind("static")
    add_files("./normal.cpp")
    add_packages("mimalloc", "loguru")
    add_options("pmem_debug")

target("pmem_test")
    add_deps("pmem")
    set_kind("binary")
    add_files("./test.c")
