add_rules("mode.debug", "mode.release")

add_requires("stb", "sokol", "handmade_math")
if is_plat("windows") then
    add_cxflags("/utf-8")
elseif is_plat("mingw") then
    add_ldflags("-static")
end

target("shadertoy")
	add_includedirs("src")
	add_files(
		"src/*.c"
	)
	add_packages("stb", "sokol", "handmade_math")
	if is_plat("windows", "mingw") then
		add_files("src/resource.rc")
		add_defines("SOKOL_D3D11")
		-- add_defines("SOKOL_GLCORE33")
	elseif is_plat("macosx") then
		add_files("src/*.mm")
		add_defines("SOKOL_METAL", "SOKOL_IMPLD")
	else
		add_defines("SOKOL_GLCORE33")
	end
