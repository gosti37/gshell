add_rules("mode.debug", "mode.release")

target("gshell")
    set_kind("binary")
    add_files("src/*.cxx", "external/linenoise/src/*.c", "external/tiny-fmt/src/*.cpp")
    add_includedirs("include", "external/linenoise/include", "external/tiny-fmt/include")
    set_optimize("fastest")