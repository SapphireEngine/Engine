--print (_ACTION)
--print (_ARGS[0])
--print (_ARGS[1])

newoption
{
	trigger     = "examples",
	description = "enable examples"
}

newoption
{
	trigger     = "gapi",
	value       = "API",
	description = "Choose a particular 3D API for rendering",
	default     = "vulkan",
	allowed =
	{
		{ "opengl",    "OpenGL" },
		{ "direct3d11",  "Direct3D 11" },
		{ "direct3d12",  "Direct3D 12" },
		{ "vulkan",  "Vulkan" }
	}
}

bSelectVS = false
bEnableExamples = false

if _ACTION == "vs2017" or _ACTION == "vs2019" then
	bSelectVS = true
end
if _OPTIONS["examples"] then
	bEnableExamples = true
end

-- Engine Lib
workspace "Engine"
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
		
include "scripts/3rdPartyLib.lua"
include "scripts/Engine.lua"
include "scripts/Test.lua"

if bEnableExamples==true then
	group "Examples/LowRender"
		include "scripts/Examples/LowRender/00.lua"
		include "scripts/Examples/LowRender/01.lua"
		include "scripts/Examples/LowRender/02.lua"	
		include "scripts/Examples/LowRender/03.lua"	
		include "scripts/Examples/LowRender/04.lua"	
		include "scripts/Examples/LowRender/05.lua"	
		include "scripts/Examples/LowRender/06.lua"	
	group ""
end