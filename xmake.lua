add_rules("mode.releasedbg", "mode.release")

add_requires("lighthook")

target("MCBEMoreDropItems")
    add_cxflags(
        "/utf-8"
    )

    set_kind("shared")
	add_defines("USE_LIGHTHOOK")
	add_defines("INCLIENT")

	add_includedirs("include")
    add_files("src/main.cpp")
	add_packages("lighthook")
	