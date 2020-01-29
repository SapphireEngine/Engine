workspace "SapphireEngine"
	location "build"
	startproject "Test"
	language "C++"
	cppdialect "C++latest"
	staticruntime "on"
	editandcontinue "Off"
	
	configurations 
	{
		"Debug",
		"Release"
	}
	
	platforms 
	{
		"x86",
		"x64"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}
	
	filter "platforms:x86"
		architecture "x86"
	filter "platforms:x64"
		architecture "x86_64"	
	
	filter "configurations:Debug"
		defines { "_DEBUG" }
		runtime "Debug"
		optimize "Debug"
		symbols "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		runtime "Release"
		optimize "Full"
	
project "Engine"
	location "build/Engine"
	kind "StaticLib"
	
	if _ACTION == "vs2017" or _ACTION == "vs2019" then
		targetdir ("$(SolutionDir)_lib/$(Configuration)/$(PlatformTarget)/")
		objdir ("!$(SolutionDir)_obj/$(Configuration)/$(PlatformTarget)/$(ProjectName)/")
		targetname "$(ProjectName)"
	end
	
	pchheader "stdafx.h"
	pchsource "src/Engine/stdafx.cpp"

	files
	{
		"src/Engine/**.h",
		"src/Engine/**.cpp",
	}
	
	includedirs
	{
		"src/3rdParty/include",
		"src/Engine"		
	}
	
project "Test"
	location "build/Test"
	kind "ConsoleApp"
	
	if _ACTION == "vs2017" or _ACTION == "vs2019" then
		targetdir ("$(SolutionDir)../bin/")
		objdir ("!$(SolutionDir)_obj/$(Configuration)/$(PlatformTarget)/$(ProjectName)/")
		targetname "$(ProjectName)_$(PlatformTarget)_$(Configuration)"
	end
	
	files
	{
		"src/Test/**.cpp",
	}
	
	includedirs
	{
		"src/3rdParty/include",
		"src/Engine",
		"src/Test"
	}
	
	if _ACTION == "vs2017" or _ACTION == "vs2019" then
	
		libdirs 
		{
			"$(SolutionDir)../src/3rdParty/Lib/$(PlatformTarget)/",
			"$(SolutionDir)_lib/$(Configuration)/$(PlatformTarget)/"
		}
	
	end
	
	dependson 
	{
		"Engine"
	}