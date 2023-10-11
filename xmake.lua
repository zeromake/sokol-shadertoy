add_rules("mode.debug", "mode.release")

add_requires("stb", "sokol", "handmade_math")
if is_plat("windows") then
    add_cxflags("/utf-8")
elseif is_plat("mingw") then
    add_ldflags("-static")
end

add_repositories("zeromake https://github.com/zeromake/xrepo.git")

rule("sokol_shader")
    set_extensions(".glsl")
    on_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local targetfile = path.relative(sourcefile, "src")
        batchcmds:mkdir("$(buildir)/sokol_shader")
        local targetfile = vformat(path.join("$(buildir)/sokol_shader", targetfile..".h"))
        batchcmds:vrunv("sokol-shdc", {
            "--ifdef",
            "-l",
            "hlsl5:glsl330:glsl300es:metal_macos:metal_ios",
            "--input",
            sourcefile,
            "--output",
            targetfile,
        })
        batchcmds:show_progress(opt.progress, "${color.build.object}glsl %s", sourcefile)
        batchcmds:add_depfiles(sourcefile)
    end)
rule_end()

add_rules("sokol_shader")
target("shader")
    set_kind("object")
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
