project "Test"
	location "../build/Test"
	kind "ConsoleApp"
	
	if bSelectVS == true then
		targetdir ("$(SolutionDir)../bin/")
		objdir ("!$(SolutionDir)_obj/$(Configuration)/$(PlatformTarget)/$(ProjectName)/")
		targetname "$(ProjectName)_$(PlatformTarget)_$(Configuration)"
	end
	
	files
	{
		"../src/Test/**.cpp",
	}
	
	includedirs
	{
		"../src/3rdParty/include",
		"../src/3rdPartyLib",
		"../src/Engine",
		"../src/Test",
		"$(VULKAN_SDK)/Include"
	}
	
	if bSelectVS == true then
	
		libdirs 
		{
			"$(SolutionDir)../src/3rdParty/Lib/",		
			"$(SolutionDir)../src/3rdParty/Lib/$(PlatformTarget)/",
			"$(SolutionDir)_lib/$(Configuration)/$(PlatformTarget)/",
			"$(VULKAN_SDK)/Lib/"
		}
	
	end
	
	dependson 
	{
		"Engine",
		"3rdPartyLib"
	}