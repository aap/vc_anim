workspace "vc_anim"
	configurations { "Release","Debug" }
	location "build"

	files { "src/*.*" }

	includedirs { "src" }
	includedirs { os.getenv("RWSDK34") }

project "vc_anim"
	kind "SharedLib"
	language "C++"
	targetname "vc_anim"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".dll"
	characterset ("MBCS")

	filter "configurations:Debug"
		defines { "DEBUG" }
		flags { "StaticRuntime" }
		symbols "On"
		debugdir "C:/Users/aap/games/gtavc"
		debugcommand "C:/Users/aap/games/gtavc/gta_vc.exe"
		postbuildcommands "copy /y \"$(TargetPath)\" \"C:\\Users\\aap\\games\\gtavc\\dlls\\vc_anim.dll\""

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		flags { "StaticRuntime" }
