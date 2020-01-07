workspace "SapphireEngine"
	location "build"
	
	configurations 
	{
		"Debug",
		"Release"
	}
	
	platforms 
	{
		"Win32",
		"Win64"
	}
	
	targetdir ("bin/%{cfg.longname}/")
	objdir ("obj/%{cfg.longname}/")
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
project "HelloWorld"
	kind "ConsoleApp"
	language "C"
	targetdir "bin/%{cfg.buildcfg}"

	files 
	{
		"src/**.h",
		"src/**.c",
		"src/**.cpp"
	}

	defines
	{
		"TEST_CORE_DEFINE"
	}
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"

	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"