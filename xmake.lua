add_rules("mode.debug", "mode.release")
add_requires("libcurl", "libevent", "nlohmann_json", "sqlite3", "openssl", "loguru")
add_includedirs("src")

-- includes("src/*/xmake.lua")

target("app_server")
    -- add_deps("quickjspp")
    set_kind("binary")
    add_files("src/*/*.cpp")
    set_languages("cxx17")
    add_packages("libevent", "libcurl", "nlohmann_json", "sqlite3", "openssl", "loguru")
    if is_plat("windows") then 
        add_cxflags("/utf-8")
    else 
        add_cxflags("-finput-charset=UTF-8")
    end 
