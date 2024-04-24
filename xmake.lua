add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    add_cxflags("/utf-8")
elseif is_plat("mingw") then
    add_ldflags("-static")
end

add_repositories("zeromake https://github.com/zeromake/xrepo.git")
add_requires("stb", "sokol", "handmade_math", "sokol-shdc")

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
	add_packages("stb", "sokol", "handmade_math")
    add_deps("shader")
    add_includedirs("$(buildir)/sokol_shader")
	if is_plat("windows", "mingw") then
		add_files("src/resource.rc")
		add_defines("SOKOL_D3D11")
		-- add_defines("SOKOL_GLCORE33")
	elseif is_plat("macosx") then
		add_files("src/*.m")
		add_defines("SOKOL_METAL")
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
