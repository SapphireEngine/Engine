project "3rdPartyLib"
	location "../build/3rdPartyLib"
	kind "StaticLib"
	
	if bSelectVS == true then
		targetdir ("$(SolutionDir)_lib/$(Configuration)/$(PlatformTarget)/")
		objdir ("!$(SolutionDir)_obj/$(Configuration)/$(PlatformTarget)/$(ProjectName)/")
		targetname "$(ProjectName)"
	end
	
	files
	{
		"../src/3rdPartyLib/**.h",
		"../src/3rdPartyLib/**.cpp",
	}
	
	includedirs
	{
		"../src/3rdParty/include",
		"../src/Engine"
	}