--print (_ACTION)
--print (_ARGS[0])
--print (_ARGS[1])

newoption
{
	trigger     = "examples",
	description = "enable examples"
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
include "scripts/Examples/00.lua"
end