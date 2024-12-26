add_rules("mode.releasedbg", "mode.release")

add_requires("hookmanager",{configs = {lighthook = true}})
add_requires("imgui v1.91.6", {configs = {dx11 = true, dx12 = true, win32 = true}})
add_rules("plugin.vsxmake.autoupdate")

target("MCBEMoreDropItems")
    add_cxflags("/utf-8")
    set_kind("shared")
	add_defines("USE_LIGHTHOOK")
	add_defines("INCLIENT")

	add_includedirs("include")
    add_files("src/main.cpp")
	add_packages("hookmanager")
	add_packages("imgui")
	