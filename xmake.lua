set_project("GShell")
set_version("1.0")

set_languages("c11", "gnu11", "cxx17")

add_rules("mode.debug", "mode.release")

if is_mode("debug") then
    set_optimize("none")
    add_defines("DEBUG")
    add_cxflags("-Wall", "-Wextra", "-Wpedantic", {tools = {"gcc", "clang"}})
    set_warnings("allextra")
else
    set_optimize("fastest")
    add_defines("NDEBUG")
end

target("tiny-fmt")
    set_kind("static")
    add_files("external/tiny-fmt/src/**.cpp")
    add_includedirs("external/tiny-fmt/include", {public = true})
target_end()

target("linenoise")
    set_kind("static")
    add_files("external/linenoise/src/**.c")
    add_includedirs("external/linenoise/include", {public = true})
target_end()

target("gshell")
    set_kind("binary")
    add_files("src/**.cpp")
    add_includedirs("include", "external/pscxx")
    add_deps("tiny-fmt", "linenoise")
target_end()