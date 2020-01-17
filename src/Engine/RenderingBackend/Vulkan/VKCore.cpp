#include "stdafx.h"
#include "VKCore.h"

#if SE_VULKAN

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
bool detail::IsExtensionAvailable(const char *extensionName, const VkExtensionPropertiesVector &vkExtensionPropertiesVector)
{
	for ( const VkExtensionProperties &vkExtensionProperties : vkExtensionPropertiesVector )
	{
		if ( strcmp(vkExtensionProperties.extensionName, extensionName) == 0 )
		{
			// The extension is available
			return true;
		}
	}

	// The extension isn't available
	return false;
}
//-----------------------------------------------------------------------------
void detail::EnumeratePhysicalDevices(VkInstance vkInstance, VkPhysicalDevices &vkPhysicalDevices)
{
	// Get the number of available physical devices
	uint32_t physicalDeviceCount = 0;
	VkResult vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
	if ( VK_SUCCESS == vkResult )
	{
		if ( physicalDeviceCount > 0 )
		{
			// Enumerate physical devices
			vkPhysicalDevices.resize(physicalDeviceCount);
			vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevices.data());
			if ( VK_SUCCESS != vkResult )
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
//-----------------------------------------------------------------------------
const char* detail::VKDebugReportObjectTypeToString(VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT)
{
#define VALUE(value) case value: return #value;

	switch ( vkDebugReportObjectTypeEXT )
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
//-----------------------------------------------------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL detail::DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char * pLayerPrefix, const char * pMessage, void * pUserData)
{
	// TODO(co) Inside e.g. the "InstancedCubes"-example the log gets currently flooded with
		//          "Warning: Vulkan debug report callback: Object type: "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT" Object: "120" Location: "5460" Message code: "0" Layer prefix: "DS" Message: "DescriptorSet 0x78 previously bound as set #0 is incompatible with set 0xc82f498 newly bound as set #0 so set #1 and any subsequent sets were disturbed by newly bound pipelineLayout (0x8b)" ".
		//          It's a known Vulkan API issue regarding validation. See https://github.com/KhronosGroup/Vulkan-Docs/issues/305 - "vkCmdBindDescriptorSets should be able to take NULL sets. #305".
		//          Currently I see no other way then ignoring this message.
	if ( VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT == objectType && 5460 == location && 0 == messageCode )
	{
		// The Vulkan call should not be aborted to have the same behavior with and without validation layers enabled
		return VK_FALSE;
	}

	if ( VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT == objectType && object && 0 == location && 0 == messageCode && nullptr != strstr(pMessage, "SteamOverlayVulkanLayer.dll") )
	{
		return VK_FALSE;
	}

	// Get log message type
	// -> Vulkan is using a flags combination, map it to our log message type enumeration
	Log::Type type = Log::Type::TRACE;
	if ( (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0 )
	{
		type = Log::Type::CRITICAL;
	}
	else if ( (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 )
	{
		type = Log::Type::WARNING;
	}
	else if ( (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0 )
	{
		type = Log::Type::PERFORMANCE_WARNING;
	}
	else if ( (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0 )
	{
		type = Log::Type::INFORMATION;
	}
	else if ( (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0 )
	{
		type = Log::Type::DEBUG;
	}

	// Construct the log message
	std::stringstream message;
	message << "Vulkan debug report callback: ";
	message << "Object type: \"" << VKDebugReportObjectTypeToString(objectType) << "\" ";
	message << "Object: \"" << object << "\" ";
	message << "Location: \"" << location << "\" ";
	message << "Message code: \"" << messageCode << "\" ";
	message << "Layer prefix: \"" << pLayerPrefix << "\" ";
	message << "Message: \"" << pMessage << "\" ";

	// Print log message
	if ( GetSubsystem<Log>().Print(type, __FILE__, static_cast<uint32_t>(__LINE__), message.str().c_str()) )
	{
		SE_DEBUG_BREAK;
	}

	// The Vulkan call should not be aborted to have the same behavior with and without validation layers enabled
	return VK_FALSE;
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================

#endif