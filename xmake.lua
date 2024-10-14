add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    add_cxflags("/utf-8")
elseif is_plat("mingw") then
    add_ldflags("-static")
end

add_repositories("zeromake https://github.com/zeromake/xrepo.git")
add_requires("stb", "sokol", "handmade_math", "sokol-shdc", "webp")
add_requires("imgui", {configs={backend="none", freetype=true}})
add_defines("USE_DBG_UI")
set_languages("c++17")
set_rundir(".")

if is_plat("windows", "mingw") then
    add_defines("SOKOL_D3D11")
	-- add_defines("SOKOL_GLCORE")
elseif is_plat("macosx") then
	-- add_defines("SOKOL_GLCORE")
    add_defines("SOKOL_METAL")
elseif is_plat("wasm") then
    add_defines("SOKOL_GLES3")
else
	add_defines("SOKOL_GLCORE")
end

target("dbgui")
    set_kind("$(kind)")
    add_packages("imgui", "sokol")
    add_files("libs/*.cc")

target("shader")
    set_kind("object")
    add_packages("sokol-shdc")
    add_rules("@sokol-shdc/shader")
    add_files("src/*.glsl")

target("shadertoy")
	add_includedirs("src")
	add_files("src/main.c")
    if not is_plat("macosx") then
        add_files("src/sokol.c")
    end
	add_packages("stb", "sokol", "handmade_math", "webp")
    add_deps("shader", "dbgui")
    add_includedirs("$(buildir)/sokol_shader", "libs")
	if is_plat("windows", "mingw") then
		add_files("src/resource.rc")
	elseif is_plat("macosx") then
		add_files("src/*.m")
        add_frameworks(
            "Metal",
            "MetalKit",
            "Appkit",
            "CoreGraphics",
            "QuartzCore"
        )
	else
        -- if is_plat("macosx") then
        --     add_frameworks(
        --         "OpenGL",
        --         "Appkit",
        --         "CoreGraphics",
        --         "QuartzCore"
        --     )
        --     add_files("src/*.m")
        -- end
		add_defines("SOKOL_GLCORE33")
	end
