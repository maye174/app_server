

target("jsc")
    set_kind("static")
    add_deps("pmem", "quickjspp")
    add_files("./jsc/*.cpp")
    set_languages("cxx20")
    add_packages("loguru")
    add_cxflags("-DJS_STRICT_NAN_BOXING")

target("ffi")
    set_kind("static")
    add_deps("jsc")
    add_files("./ffi/*.cpp")
    set_languages("cxx20")
    add_packages("loguru")
    add_cxflags("-DJS_STRICT_NAN_BOXING")

target("dynamic_test")
    add_deps("jsc")
    set_kind("shared")
    add_files("test/test_dynamic.c")
    add_cxflags("-DJS_STRICT_NAN_BOXING")

target("js_test")
    set_kind("binary")
    add_deps("jsc", "ffi")
    add_files("test/test.c")
    after_build(function (target)
        os.cp("$(projectdir)/src/js/test/*.js", target:targetdir())
    end)

