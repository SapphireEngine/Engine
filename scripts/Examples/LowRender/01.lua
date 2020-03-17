project "LowRender_01"
	location "../../../build/ExampleLowRender/01"
	kind "ConsoleApp"
	
	if bSelectVS == true then
		targetdir ("$(SolutionDir)../bin/")
		objdir ("!$(SolutionDir)_obj/$(Configuration)/$(PlatformTarget)/$(ProjectName)/")
		targetname "$(ProjectName)_$(PlatformTarget)_$(Configuration)"
	end
	
	files
	{
		"../../../examples/LowRender/01/**.cpp",
		"../../../examples/LowRender/01/**.h",
	}
	
	includedirs
	{
		"../../../src/3rdParty/include",
		"../../../src/3rdPartyLib",
		"../../../src/Engine",
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