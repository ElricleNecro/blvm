workspace "BLVM"
	architecture "x86_64"
	language "C"
	cdialect "C11"

	configurations { "Debug", "Release" }
		includedirs { "include/", }

		warnings "Extra"
		enablewarnings {
			"switch-enum",
			"conversion",
		}

	configurations "Release"
		optimize "Full"

project("ParseArgsC")
	language("C")
	kind("SharedLib")

	location("build/")
	targetdir("build/%{cfg.buildcfg}/lib")

	includedirs {
		".submodule/ParseArgsC/include/",
	}

	files(
		{
			".submodule/%{prj.name}/src/Parser.c"
		}
	)

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

project "blvm"
	filename ".blvm"
	kind "SharedLib"

	location "build/"
	targetdir "build/%{cfg.buildcfg}/lib"

	files { "src/%{prj.name}/**.c" }

	includedirs {
		"include/%{prj.name}",
		".submodule/ParseArgsC/include/",
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "system:linux"
		systemversion "latest"
		pic "On"
		--staticruntime "On"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		optimize "On"
		flags { "LinkTimeOptimization", "FatalWarnings" }

project "blvi"
	filename ".vm"
	kind "ConsoleApp"

	location "build/"
	targetdir "build/%{cfg.buildcfg}/bin"

	files { "src/%{prj.name}/**.c" }

	includedirs {
		"include/blvm",
		"include/%{prj.name}",
		".submodule/ParseArgsC/include/",
	}

	links {
		"blvm",
		"ParseArgsC",
	}

	libdirs {
		"build/blvm/lib",
		"build/ParseArgsC/lib",
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "system:linux"
		systemversion "latest"
		pic "On"
		--staticruntime "On"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		optimize "On"
		flags { "LinkTimeOptimization", "FatalWarnings" }

project "blasm"
	filename ".asm"
	kind "ConsoleApp"

	location "build/"
	targetdir "build/%{cfg.buildcfg}/bin"

	files { "src/%{prj.name}/**.c" }

	includedirs {
		"include/blvm",
		"include/%{prj.name}",
		".submodule/ParseArgsC/include/",
	}

	links {
		"blvm",
		"ParseArgsC",
	}

	libdirs {
		"build/blvm/lib",
		"build/ParseArgsC/lib",
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "system:linux"
		systemversion "latest"
		pic "On"
		--staticruntime "On"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		optimize "On"
		flags { "LinkTimeOptimization", "FatalWarnings" }

project "deblasm"
	filename ".deasm"
	kind "ConsoleApp"

	location "build/"
	targetdir "build/%{cfg.buildcfg}/bin"

	files { "src/%{prj.name}/**.c" }

	includedirs {
		"include/blvm",
		"include/%{prj.name}",
		".submodule/ParseArgsC/include/",
	}

	links {
		"blvm",
		"ParseArgsC",
	}

	libdirs {
		"build/blvm/lib",
		"build/ParseArgsC/lib",
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "system:linux"
		systemversion "latest"
		pic "On"
		--staticruntime "On"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		optimize "On"
		flags { "LinkTimeOptimization", "FatalWarnings" }
