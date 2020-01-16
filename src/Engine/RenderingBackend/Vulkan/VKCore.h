#pragma once

#include "Core/Memory/Memory.h"
#include "Core/Debug/Log.h"
#include "RenderingBackend/Vulkan/VKHeader.h"

//=============================================================================
SE_NAMESPACE_BEGIN

#if SE_VULKAN

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
		32,		///< MaxLights
		6,		///< MaxClipPlanes
		32,		///< MaxTextureUnits
		32,		///< MaxTextureCoords
		64,		///< MaxVertexAttribs
		4096,	///< MaxVertexUniformComponents
		64,		///< MaxVaryingFloats
		32,		///< MaxVertexTextureImageUnits
		80,		///< MaxCombinedTextureImageUnits
		32,		///< MaxTextureImageUnits
		4096,	///< MaxFragmentUniformComponents
		32,		///< MaxDrawBuffers
		128,	///< MaxVertexUniformVectors
		8,		///< MaxVaryingVectors
		16,		///< MaxFragmentUniformVectors
		16,		///< MaxVertexOutputVectors
		15,		///< MaxFragmentInputVectors
		-8,		///< MinProgramTexelOffset
		7,		///< MaxProgramTexelOffset
		8,		///< MaxClipDistances
		65535,	///< MaxComputeWorkGroupCountX
		65535,	///< MaxComputeWorkGroupCountY
		65535,	///< MaxComputeWorkGroupCountZ
		1024,	///< MaxComputeWorkGroupSizeX
		1024,	///< MaxComputeWorkGroupSizeY
		64,		///< MaxComputeWorkGroupSizeZ
		1024,	///< MaxComputeUniformComponents
		16,		///< MaxComputeTextureImageUnits
		8,		///< MaxComputeImageUniforms
		8,		///< MaxComputeAtomicCounters
		1,		///< MaxComputeAtomicCounterBuffers
		60,		///< MaxVaryingComponents
		64,		///< MaxVertexOutputComponents
		64,		///< MaxGeometryInputComponents
		128,	///< MaxGeometryOutputComponents
		128,	///< MaxFragmentInputComponents
		8,		///< MaxImageUnits
		8,		///< MaxCombinedImageUnitsAndFragmentOutputs
		8,		///< MaxCombinedShaderOutputResources
		0,		///< MaxImageSamples
		0,		///< MaxVertexImageUniforms
		0,		///< MaxTessControlImageUniforms
		0,		///< MaxTessEvaluationImageUniforms
		0,		///< MaxGeometryImageUniforms
		8,		///< MaxFragmentImageUniforms
		8,		///< MaxCombinedImageUniforms
		16,		///< MaxGeometryTextureImageUnits
		256,	///< MaxGeometryOutputVertices
		1024,	///< MaxGeometryTotalOutputComponents
		1024,	///< MaxGeometryUniformComponents
		64,		///< MaxGeometryVaryingComponents
		128,	///< MaxTessControlInputComponents
		128,	///< MaxTessControlOutputComponents
		16,		///< MaxTessControlTextureImageUnits
		1024,	///< MaxTessControlUniformComponents
		4096,	///< MaxTessControlTotalOutputComponents
		128,	///< MaxTessEvaluationInputComponents
		128,	///< MaxTessEvaluationOutputComponents
		16,		///< MaxTessEvaluationTextureImageUnits
		1024,	///< MaxTessEvaluationUniformComponents
		120,	///< MaxTessPatchComponents
		32,		///< MaxPatchVertices
		64,		///< MaxTessGenLevel
		16,		///< MaxViewports
		0,		///< MaxVertexAtomicCounters
		0,		///< MaxTessControlAtomicCounters
		0,		///< MaxTessEvaluationAtomicCounters
		0,		///< MaxGeometryAtomicCounters
		8,		///< MaxFragmentAtomicCounters
		8,		///< MaxCombinedAtomicCounters
		1,		///< MaxAtomicCounterBindings
		0,		///< MaxVertexAtomicCounterBuffers
		0,		///< MaxTessControlAtomicCounterBuffers
		0,		///< MaxTessEvaluationAtomicCounterBuffers
		0,		///< MaxGeometryAtomicCounterBuffers
		1,		///< MaxFragmentAtomicCounterBuffers
		1,		///< MaxCombinedAtomicCounterBuffers
		16384,	///< MaxAtomicCounterBufferSize
		4,		///< MaxTransformFeedbackBuffers
		64,		///< MaxTransformFeedbackInterleavedComponents
		8,		///< MaxCullDistances
		8,		///< MaxCombinedClipAndCullDistances
		4,		///< MaxSamples
		256,	///< MaxMeshOutputVerticesNV
		512,	///< MaxMeshOutputPrimitivesNV
		32,		///< MaxMeshWorkGroupSizeX_NV
		1,		///< MaxMeshWorkGroupSizeY_NV
		1,		///< MaxMeshWorkGroupSizeZ_NV
		32,		///< MaxTaskWorkGroupSizeX_NV
		1,		///< MaxTaskWorkGroupSizeY_NV
		1,		///< MaxTaskWorkGroupSizeZ_NV
		4,		///< MaxMeshViewCountNV
		{		///< limits
			1,	///< nonInductiveForLoops
			1,	///< whileLoops
			1,	///< doWhileLoops
			1,	///< generalUniformIndexing
			1,	///< generalAttributeMatrixVectorIndexing
			1,	///< generalVaryingIndexing
			1,	///< generalSamplerIndexing
			1,	///< generalVariableIndexing
			1,	///< generalConstantMatrixVectorIndexing
		}
	};
#endif

	[[nodiscard]] bool IsExtensionAvailable(const char *extensionName, const VkExtensionPropertiesVector &vkExtensionPropertiesVector)
	{
		for( const VkExtensionProperties &vkExtensionProperties : vkExtensionPropertiesVector )
		{
			if( strcmp(vkExtensionProperties.extensionName, extensionName) == 0 )
			{
				// The extension is available
				return true;
			}
		}

		// The extension isn't available
		return false;
	}

	void EnumeratePhysicalDevices(VkInstance vkInstance, VkPhysicalDevices &vkPhysicalDevices)
	{
		// Get the number of available physical devices
		uint32_t physicalDeviceCount = 0;
		VkResult vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
		if( VK_SUCCESS == vkResult )
		{
			if( physicalDeviceCount > 0 )
			{
				// Enumerate physical devices
				vkPhysicalDevices.resize(physicalDeviceCount);
				vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevices.data());
				if( VK_SUCCESS != vkResult )
				{
					// Error!
					SE_LOG(CRITICAL, "Failed to enumerate physical Vulkan devices")
				}
			}
			else
			{
				// Error!
				SE_LOG(CRITICAL, "There are no physical Vulkan devices")
			}
		}
		else
		{
			// Error!
			SE_LOG(CRITICAL, "Failed to get the number of physical Vulkan devices")
		}
	}

	[[nodiscard]] const char* vkDebugReportObjectTypeToString(VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT)
	{
#define VALUE(value) case value: return #value;

		switch( vkDebugReportObjectTypeEXT )
		{
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT)
		//	VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT)	not possible
		//	VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT)	not possible	
		//	VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT)	not possible
		//	VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_KHR_EXT)	not possible
		//	VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_BEGIN_RANGE_EXT)	not possible
		//	VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_END_RANGE_EXT)	not possible
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_RANGE_SIZE_EXT)
		VALUE(VK_DEBUG_REPORT_OBJECT_TYPE_MAX_ENUM_EXT)
		}
#undef VALUE

		return nullptr;
	}

	[[nodiscard]] VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void *pUserData)
	{
		// TODO(co) Inside e.g. the "InstancedCubes"-example the log gets currently flooded with
		//          "Warning: Vulkan debug report callback: Object type: "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT" Object: "120" Location: "5460" Message code: "0" Layer prefix: "DS" Message: "DescriptorSet 0x78 previously bound as set #0 is incompatible with set 0xc82f498 newly bound as set #0 so set #1 and any subsequent sets were disturbed by newly bound pipelineLayout (0x8b)" ".
		//          It's a known Vulkan API issue regarding validation. See https://github.com/KhronosGroup/Vulkan-Docs/issues/305 - "vkCmdBindDescriptorSets should be able to take NULL sets. #305".
		//          Currently I see no other way then ignoring this message.
		if( VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT == objectType && 5460 == location && 0 == messageCode )
		{
			// The Vulkan call should not be aborted to have the same behavior with and without validation layers enabled
			return VK_FALSE;
		}

		if( VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT == objectType && object && 0 == location && 0 == messageCode && nullptr != strstr(pMessage, "SteamOverlayVulkanLayer.dll") )
		{
			return VK_FALSE;
		}

		// Get log message type
		// -> Vulkan is using a flags combination, map it to our log message type enumeration
		Log::Type type = Log::Type::TRACE;
		if( (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0 )
		{
			type = Log::Type::CRITICAL;
		}
		else if( (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 )
		{
			type = Log::Type::WARNING;
		}
		else if( (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0 )
		{
			type = Log::Type::PERFORMANCE_WARNING;
		}
		else if( (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0 )
		{
			type = Log::Type::INFORMATION;
		}
		else if( (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0 )
		{
			type = Log::Type::DEBUG;
		}

		// Construct the log message
		std::stringstream message;
		message << "Vulkan debug report callback: ";
		message << "Object type: \"" << vkDebugReportObjectTypeToString(objectType) << "\" ";
		message << "Object: \"" << object << "\" ";
		message << "Location: \"" << location << "\" ";
		message << "Message code: \"" << messageCode << "\" ";
		message << "Layer prefix: \"" << pLayerPrefix << "\" ";
		message << "Message: \"" << pMessage << "\" ";

		// Print log message
		if( GetSubsystem<Log>().Print(type, __FILE__, static_cast<uint32_t>(__LINE__), message.str().c_str()) )
		{
			SE_DEBUG_BREAK;
		}

		// The Vulkan call should not be aborted to have the same behavior with and without validation layers enabled
		return VK_FALSE;
	}

	[[nodiscard]] VKAPI_ATTR void* VKAPI_CALL vkAllocationFunction(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope)
	{
		return reinterpret_cast<IAllocator*>(pUserData)->Reallocate(nullptr, 0, size, alignment);
	}

	[[nodiscard]] VKAPI_ATTR void* VKAPI_CALL vkReallocationFunction(void *pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope)
	{
		return reinterpret_cast<IAllocator*>(pUserData)->Reallocate(pOriginal, 0, size, alignment);
	}

	VKAPI_ATTR void VKAPI_CALL vkFreeFunction(void *pUserData, void *pMemory)
	{
		reinterpret_cast<IAllocator*>(pUserData)->Reallocate(pMemory, 0, 0, 1);
	}
}

#endif

SE_NAMESPACE_END
//=============================================================================