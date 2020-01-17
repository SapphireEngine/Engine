#pragma once

#if SE_VULKAN

#include "Core/Memory/Memory.h"
#include "Core/Debug/Log.h"
#include "RenderingBackend/Vulkan/VKHeader.h"

//=============================================================================
SE_NAMESPACE_BEGIN

namespace detail
{
	static constexpr const char *GLSL_NAME = "GLSL";	// ASCII name of this shader language, always valid (do not free the memory the returned pointer is pointing to)
	typedef std::vector<VkPhysicalDevice> VkPhysicalDevices;
	typedef std::vector<VkExtensionProperties> VkExtensionPropertiesVector;
	typedef std::array<VkPipelineShaderStageCreateInfo, 5> VkPipelineShaderStageCreateInfos;

#if SE_PLATFORM_ANDROID
	// On Android we need to explicitly select all layers
	static constexpr uint32_t NUMBER_OF_VALIDATION_LAYERS = 6;
	static constexpr const char *VALIDATION_LAYER_NAMES[] =
	{
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_GOOGLE_unique_objects"
	};
#else
	// On desktop the LunarG loaders exposes a meta layer that contains all layers
	static constexpr uint32_t NUMBER_OF_VALIDATION_LAYERS = 1;
	static constexpr const char* VALIDATION_LAYER_NAMES[] =
	{
		"VK_LAYER_LUNARG_standard_validation"
	};
#endif

#ifdef SE_VULKAN_GLSLTOSPIRV
	static bool GlslangInitialized = false;

	// Settings from "glslang/StandAlone/ResourceLimits.cpp"
	static constexpr TBuiltInResource DefaultTBuiltInResource =
	{
		32,		// MaxLights
		6,		// MaxClipPlanes
		32,		// MaxTextureUnits
		32,		// MaxTextureCoords
		64,		// MaxVertexAttribs
		4096,	// MaxVertexUniformComponents
		64,		// MaxVaryingFloats
		32,		// MaxVertexTextureImageUnits
		80,		// MaxCombinedTextureImageUnits
		32,		// MaxTextureImageUnits
		4096,	// MaxFragmentUniformComponents
		32,		// MaxDrawBuffers
		128,	// MaxVertexUniformVectors
		8,		// MaxVaryingVectors
		16,		// MaxFragmentUniformVectors
		16,		// MaxVertexOutputVectors
		15,		// MaxFragmentInputVectors
		-8,		// MinProgramTexelOffset
		7,		// MaxProgramTexelOffset
		8,		// MaxClipDistances
		65535,	// MaxComputeWorkGroupCountX
		65535,	// MaxComputeWorkGroupCountY
		65535,	// MaxComputeWorkGroupCountZ
		1024,	// MaxComputeWorkGroupSizeX
		1024,	// MaxComputeWorkGroupSizeY
		64,		// MaxComputeWorkGroupSizeZ
		1024,	// MaxComputeUniformComponents
		16,		// MaxComputeTextureImageUnits
		8,		// MaxComputeImageUniforms
		8,		// MaxComputeAtomicCounters
		1,		// MaxComputeAtomicCounterBuffers
		60,		// MaxVaryingComponents
		64,		// MaxVertexOutputComponents
		64,		// MaxGeometryInputComponents
		128,	// MaxGeometryOutputComponents
		128,	// MaxFragmentInputComponents
		8,		// MaxImageUnits
		8,		// MaxCombinedImageUnitsAndFragmentOutputs
		8,		// MaxCombinedShaderOutputResources
		0,		// MaxImageSamples
		0,		// MaxVertexImageUniforms
		0,		// MaxTessControlImageUniforms
		0,		// MaxTessEvaluationImageUniforms
		0,		// MaxGeometryImageUniforms
		8,		// MaxFragmentImageUniforms
		8,		// MaxCombinedImageUniforms
		16,		// MaxGeometryTextureImageUnits
		256,	// MaxGeometryOutputVertices
		1024,	// MaxGeometryTotalOutputComponents
		1024,	// MaxGeometryUniformComponents
		64,		// MaxGeometryVaryingComponents
		128,	// MaxTessControlInputComponents
		128,	// MaxTessControlOutputComponents
		16,		// MaxTessControlTextureImageUnits
		1024,	// MaxTessControlUniformComponents
		4096,	// MaxTessControlTotalOutputComponents
		128,	// MaxTessEvaluationInputComponents
		128,	// MaxTessEvaluationOutputComponents
		16,		// MaxTessEvaluationTextureImageUnits
		1024,	// MaxTessEvaluationUniformComponents
		120,	// MaxTessPatchComponents
		32,		// MaxPatchVertices
		64,		// MaxTessGenLevel
		16,		// MaxViewports
		0,		// MaxVertexAtomicCounters
		0,		// MaxTessControlAtomicCounters
		0,		// MaxTessEvaluationAtomicCounters
		0,		// MaxGeometryAtomicCounters
		8,		// MaxFragmentAtomicCounters
		8,		// MaxCombinedAtomicCounters
		1,		// MaxAtomicCounterBindings
		0,		// MaxVertexAtomicCounterBuffers
		0,		// MaxTessControlAtomicCounterBuffers
		0,		// MaxTessEvaluationAtomicCounterBuffers
		0,		// MaxGeometryAtomicCounterBuffers
		1,		// MaxFragmentAtomicCounterBuffers
		1,		// MaxCombinedAtomicCounterBuffers
		16384,	// MaxAtomicCounterBufferSize
		4,		// MaxTransformFeedbackBuffers
		64,		// MaxTransformFeedbackInterleavedComponents
		8,		// MaxCullDistances
		8,		// MaxCombinedClipAndCullDistances
		4,		// MaxSamples
		256,	// MaxMeshOutputVerticesNV
		512,	// MaxMeshOutputPrimitivesNV
		32,		// MaxMeshWorkGroupSizeX_NV
		1,		// MaxMeshWorkGroupSizeY_NV
		1,		// MaxMeshWorkGroupSizeZ_NV
		32,		// MaxTaskWorkGroupSizeX_NV
		1,		// MaxTaskWorkGroupSizeY_NV
		1,		// MaxTaskWorkGroupSizeZ_NV
		4,		// MaxMeshViewCountNV
		{		// limits
			1,	// nonInductiveForLoops
			1,	// whileLoops
			1,	// doWhileLoops
			1,	// generalUniformIndexing
			1,	// generalAttributeMatrixVectorIndexing
			1,	// generalVaryingIndexing
			1,	// generalSamplerIndexing
			1,	// generalVariableIndexing
			1,	// generalConstantMatrixVectorIndexing
		}
	};
#endif
	
	[[nodiscard]] bool IsExtensionAvailable(const char *extensionName, const VkExtensionPropertiesVector &vkExtensionPropertiesVector);

	void EnumeratePhysicalDevices(VkInstance vkInstance, VkPhysicalDevices &vkPhysicalDevices);

	[[nodiscard]] const char* VKDebugReportObjectTypeToString(VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT);


	[[nodiscard]] VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void *pUserData);

	[[nodiscard]] inline VKAPI_ATTR void* VKAPI_CALL VKAllocationFunction(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope)
	{
		return reinterpret_cast<IAllocator*>(pUserData)->Reallocate(nullptr, 0, size, alignment);
	}

	[[nodiscard]] inline VKAPI_ATTR void* VKAPI_CALL VKReallocationFunction(void *pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope)
	{
		return reinterpret_cast<IAllocator*>(pUserData)->Reallocate(pOriginal, 0, size, alignment);
	}

	VKAPI_ATTR  inline void VKAPI_CALL VKFreeFunction(void *pUserData, void *pMemory)
	{
		reinterpret_cast<IAllocator*>(pUserData)->Reallocate(pMemory, 0, 0, 1);
	}
}

SE_NAMESPACE_END
//=============================================================================

#endif