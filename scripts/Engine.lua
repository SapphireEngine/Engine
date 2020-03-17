project "Engine"
	location "../build/Engine"
	kind "StaticLib"
	
	if bSelectVS == true then
		targetdir ("$(SolutionDir)_lib/$(Configuration)/$(PlatformTarget)/")
		objdir ("!$(SolutionDir)_obj/$(Configuration)/$(PlatformTarget)/$(ProjectName)/")
		targetname "$(ProjectName)"
	end
	
	pchheader "stdafx.h"
	pchsource "../src/Engine/stdafx.cpp"

	files
	{
		"../src/Engine/**.h",
		"../src/Engine/**.cpp",
	}
	
	includedirs
	{
		"../src/3rdParty/include",
		"../src/3rdPartyLib",
		"../src/Engine",
		"$(VULKAN_SDK)/Include"
	}