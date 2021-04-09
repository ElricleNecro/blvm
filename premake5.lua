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

project "blasm2nasm"
	filename ".asm_nasm"
	kind "ConsoleApp"

	location "build/"
	targetdir "build/%{cfg.buildcfg}/bin"

	files { "src/%{prj.name}/**.c", "asm/*", "include/blvm/**.h", "include/%{prj.name}/**.h" }

	links {
		"blvm",
		"ParseArgsC",
	}

	libdirs {
		"build/blvm/lib",
		"build/ParseArgsC/lib",
	}

	filter 'files:**.nasm'
		-- A message to display while this build step is running (optional)
		buildmessage 'Compiling %{file.relpath}'

		-- One or more commands to run (required)
		buildcommands {
			'xxd -i "%{file.relpath}" > "../include/%{prj.name}/generated/%{file.basename}.h"',
		}

		-- One or more outputs resulting from the build (required)
		buildoutputs { "../include/%{prj.name}/generated/%{file.basename}.h" }

		-- One or more additional dependencies for this build command (optional)
		--buildinputs { 'path/to/file1.ext', 'path/to/file2.ext' }

	filter {}

	includedirs {
		"include/blvm",
		"include/%{prj.name}",
		".submodule/ParseArgsC/include/",
		"build/%{cfg.buildcfg}/include/",
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
