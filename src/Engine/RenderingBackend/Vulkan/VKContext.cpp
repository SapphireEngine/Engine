#include "stdafx.h"
#include "VKContext.h"
#include "VKCore.h"
#include "VKRuntimeLinking.h"
#include "Core/Debug/Log.h"

#if SE_VULKAN

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
[[nodiscard]] VkPhysicalDevice selectPhysicalDevice(const detail::VkPhysicalDevices& vkPhysicalDevices, bool validationEnabled, bool& enableDebugMarker)
{
	// TODO(co) I'am sure this selection can be improved (rating etc.)
	for( const VkPhysicalDevice &vkPhysicalDevice : vkPhysicalDevices )
	{
		// Get of device extensions
		uint32_t propertyCount = 0;
		if( (vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &propertyCount, nullptr) != VK_SUCCESS) || (0 == propertyCount) )
		{
			// Reject physical Vulkan device
			continue;
		}
		detail::VkExtensionPropertiesVector vkExtensionPropertiesVector(propertyCount);
		if( vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &propertyCount, vkExtensionPropertiesVector.data()) != VK_SUCCESS )
		{
			// Reject physical Vulkan device
			continue;
		}

		{ // Reject physical Vulkan devices basing on swap chain support
			// Check device extensions
			static constexpr std::array<const char*, 2> deviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
				VK_KHR_MAINTENANCE1_EXTENSION_NAME	// We want to be able to specify a negative viewport height, this way we don't have to apply "<output position>.y = -<output position>.y" inside vertex shaders to compensate for the Vulkan coordinate system
			};
			bool rejectDevice = false;
			for( const char* deviceExtension : deviceExtensions )
			{
				if( !detail::IsExtensionAvailable(deviceExtension, vkExtensionPropertiesVector) )
				{
					rejectDevice = true;
					break;
				}
			}
			if( rejectDevice )
			{
				// Reject physical Vulkan device
				continue;
			}
		}

		{ // Reject physical Vulkan devices basing on supported API version and some basic limits
			VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
			vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkPhysicalDeviceProperties);
			const uint32_t majorVersion = VK_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
			if( (majorVersion < 1) || (vkPhysicalDeviceProperties.limits.maxImageDimension2D < 4096) )
			{
				// Reject physical Vulkan device
				continue;
			}
		}

		// Reject physical Vulkan devices basing on supported queue family
		uint32_t queueFamilyPropertyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
		if( 0 == queueFamilyPropertyCount )
		{
			// Reject physical Vulkan device
			continue;
		}
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
		for( uint32_t i = 0; i < queueFamilyPropertyCount; ++i )
		{
			if( (queueFamilyProperties[i].queueCount > 0) && (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) )
			{
				// Check whether or not the "VK_EXT_debug_marker"-extension is available
				// -> The "VK_EXT_debug_marker"-extension is only available when the application gets started by tools like RenderDoc ( https://renderdoc.org/ )
				// -> See "Offline debugging in Vulkan with VK_EXT_debug_marker and RenderDoc" - https://www.saschawillems.de/?page_id=2017
				if( enableDebugMarker )
				{
					// Check whether or not the "VK_EXT_debug_marker"-extension is available
					if( detail::IsExtensionAvailable(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, vkExtensionPropertiesVector) )
					{
						// TODO(co) Currently, when trying to use RenderDoc ( https://renderdoc.org/ ) while having Vulkan debug layers enabled, RenderDoc crashes
						// -> Windows 10 x64
						// -> Radeon software 17.7.2
						// -> GPU: AMD 290X
						// -> LunarG® Vulkan™ SDK 1.0.54.0
						// -> Tried RenderDoc v0.91 as well as "Nightly v0.x @ 2017-08-21 (Win x64)" ("RenderDoc_2017_08_21_177d595d_64.zip")
						if( validationEnabled )
						{
							enableDebugMarker = false;
							SE_LOG(WARNING, "Vulkan validation layers are enabled: If you want to use debug markers (\"VK_EXT_debug_marker\"-extension) please disable the validation layers");
						}
					}
					else
					{
						// Silently disable debug marker
						enableDebugMarker = false;
					}
				}

				// Select physical Vulkan device
				return vkPhysicalDevice;
			}
		}
	}

	// Error!
	SE_LOG(CRITICAL, "Failed to select a physical Vulkan device");
	return VK_NULL_HANDLE;
}
//-----------------------------------------------------------------------------
[[nodiscard]] VkResult createVkDevice(const VkAllocationCallbacks *vkAllocationCallbacks, VkPhysicalDevice vkPhysicalDevice, const VkDeviceQueueCreateInfo &vkDeviceQueueCreateInfo, bool enableValidation, bool enableDebugMarker, VkDevice &vkDevice)
{
	// See http://vulkan.gpuinfo.org/listfeatures.php to check out GPU hardware capabilities
	static constexpr std::array<const char*, 3> enabledExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME,	// We want to be able to specify a negative viewport height, this way we don't have to apply "<output position>.y = -<output position>.y" inside vertex shaders to compensate for the Vulkan coordinate system
		VK_EXT_DEBUG_MARKER_EXTENSION_NAME
	};
	static constexpr VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures =
	{
		VK_FALSE,	// robustBufferAccess (VkBool32)
		VK_FALSE,	// fullDrawIndexUint32 (VkBool32)
		VK_TRUE,	// imageCubeArray (VkBool32)
		VK_FALSE,	// independentBlend (VkBool32)
		VK_TRUE,	// geometryShader (VkBool32)
		VK_TRUE,	// tessellationShader (VkBool32)
		VK_FALSE,	// sampleRateShading (VkBool32)
		VK_FALSE,	// dualSrcBlend (VkBool32)
		VK_FALSE,	// logicOp (VkBool32)
		VK_TRUE,	// multiDrawIndirect (VkBool32)
		VK_FALSE,	// drawIndirectFirstInstance (VkBool32)
		VK_TRUE,	// depthClamp (VkBool32)
		VK_FALSE,	// depthBiasClamp (VkBool32)
		VK_TRUE,	// fillModeNonSolid (VkBool32)
		VK_FALSE,	// depthBounds (VkBool32)
		VK_FALSE,	// wideLines (VkBool32)
		VK_FALSE,	// largePoints (VkBool32)
		VK_FALSE,	// alphaToOne (VkBool32)
		VK_FALSE,	// multiViewport (VkBool32)
		VK_TRUE,	// samplerAnisotropy (VkBool32)
		VK_FALSE,	// textureCompressionETC2 (VkBool32)
		VK_FALSE,	// textureCompressionASTC_LDR (VkBool32)
		VK_TRUE,	// textureCompressionBC (VkBool32)
		VK_TRUE,	// occlusionQueryPrecise (VkBool32)
		VK_TRUE,	// pipelineStatisticsQuery (VkBool32)
		VK_FALSE,	// vertexPipelineStoresAndAtomics (VkBool32)
		VK_FALSE,	// fragmentStoresAndAtomics (VkBool32)
		VK_FALSE,	// shaderTessellationAndGeometryPointSize (VkBool32)
		VK_FALSE,	// shaderImageGatherExtended (VkBool32)
		VK_FALSE,	// shaderStorageImageExtendedFormats (VkBool32)
		VK_FALSE,	// shaderStorageImageMultisample (VkBool32)
		VK_FALSE,	// shaderStorageImageReadWithoutFormat (VkBool32)
		VK_FALSE,	// shaderStorageImageWriteWithoutFormat (VkBool32)
		VK_FALSE,	// shaderUniformBufferArrayDynamicIndexing (VkBool32)
		VK_FALSE,	// shaderSampledImageArrayDynamicIndexing (VkBool32)
		VK_FALSE,	// shaderStorageBufferArrayDynamicIndexing (VkBool32)
		VK_FALSE,	// shaderStorageImageArrayDynamicIndexing (VkBool32)
		VK_FALSE,	// shaderClipDistance (VkBool32)
		VK_FALSE,	// shaderCullDistance (VkBool32)
		VK_FALSE,	// shaderFloat64 (VkBool32)
		VK_FALSE,	// shaderInt64 (VkBool32)
		VK_FALSE,	// shaderInt16 (VkBool32)
		VK_FALSE,	// shaderResourceResidency (VkBool32)
		VK_FALSE,	// shaderResourceMinLod (VkBool32)
		VK_FALSE,	// sparseBinding (VkBool32)
		VK_FALSE,	// sparseResidencyBuffer (VkBool32)
		VK_FALSE,	// sparseResidencyImage2D (VkBool32)
		VK_FALSE,	// sparseResidencyImage3D (VkBool32)
		VK_FALSE,	// sparseResidency2Samples (VkBool32)
		VK_FALSE,	// sparseResidency4Samples (VkBool32)
		VK_FALSE,	// sparseResidency8Samples (VkBool32)
		VK_FALSE,	// sparseResidency16Samples (VkBool32)
		VK_FALSE,	// sparseResidencyAliased (VkBool32)
		VK_FALSE,	// variableMultisampleRate (VkBool32)
		VK_FALSE	// inheritedQueries (VkBool32)
	};
	const VkDeviceCreateInfo vkDeviceCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,							// sType (VkStructureType)
		nullptr,														// pNext (const void*)
		0,																// flags (VkDeviceCreateFlags)
		1,																// queueCreateInfoCount (uint32_t)
		&vkDeviceQueueCreateInfo,										// pQueueCreateInfos (const VkDeviceQueueCreateInfo*)
		enableValidation ? detail::NUMBER_OF_VALIDATION_LAYERS : 0,		// enabledLayerCount (uint32_t)
		enableValidation ? detail::VALIDATION_LAYER_NAMES : nullptr,	// ppEnabledLayerNames (const char* const*)
		enableDebugMarker ? 3u : 2u,									// enabledExtensionCount (uint32_t)
		enabledExtensions.empty() ? nullptr : enabledExtensions.data(),	// ppEnabledExtensionNames (const char* const*)
		&vkPhysicalDeviceFeatures										// pEnabledFeatures (const VkPhysicalDeviceFeatures*)
	};
	const VkResult vkResult = vkCreateDevice(vkPhysicalDevice, &vkDeviceCreateInfo, vkAllocationCallbacks, &vkDevice);
	if( VK_SUCCESS == vkResult && enableDebugMarker )
	{
		// Get "VK_EXT_debug_marker"-extension function pointers

#define IMPORT_FUNC(funcName) \
		funcName = reinterpret_cast<PFN_##funcName>(vkGetDeviceProcAddr(vkDevice, #funcName)); \
		if (nullptr == funcName) \
		{	 \
			SE_LOG(CRITICAL, "Failed to load instance based Vulkan function pointer \"%s\"", #funcName) \
		}
		// "VK_EXT_debug_marker"-extension
		IMPORT_FUNC(vkDebugMarkerSetObjectTagEXT)
		IMPORT_FUNC(vkDebugMarkerSetObjectNameEXT)
		IMPORT_FUNC(vkCmdDebugMarkerBeginEXT)
		IMPORT_FUNC(vkCmdDebugMarkerEndEXT)
		IMPORT_FUNC(vkCmdDebugMarkerInsertEXT)

#undef IMPORT_FUNC
	}

	return vkResult;
}
//-----------------------------------------------------------------------------
[[nodiscard]] VkDevice createVkDevice(const VkAllocationCallbacks* vkAllocationCallbacks, VkPhysicalDevice vkPhysicalDevice, bool enableValidation, bool enableDebugMarker, uint32_t& graphicsQueueFamilyIndex, uint32_t& presentQueueFamilyIndex)
{
	VkDevice vkDevice = VK_NULL_HANDLE;

	// Get physical device queue family properties
	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
	if( queueFamilyPropertyCount > 0 )
	{
		std::vector<VkQueueFamilyProperties> vkQueueFamilyProperties;
		vkQueueFamilyProperties.resize(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, vkQueueFamilyProperties.data());

		// Find a queue that supports graphics operations
		uint32_t graphicsQueueIndex = 0;
		for( ; graphicsQueueIndex < queueFamilyPropertyCount; ++graphicsQueueIndex )
		{
			if( vkQueueFamilyProperties[graphicsQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT )
			{
				// Create logical Vulkan device instance
				static constexpr std::array<float, 1> queuePriorities = { 0.0f };
				const VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo =
				{
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,	// sType (VkStructureType)
					nullptr,									// pNext (const void*)
					0,											// flags (VkDeviceQueueCreateFlags)
					graphicsQueueIndex,							// queueFamilyIndex (uint32_t)
					1,											// queueCount (uint32_t)
					queuePriorities.data()						// pQueuePriorities (const float*)
				};
				VkResult vkResult = createVkDevice(vkAllocationCallbacks, vkPhysicalDevice, vkDeviceQueueCreateInfo, enableValidation, enableDebugMarker, vkDevice);
				if( VK_ERROR_LAYER_NOT_PRESENT == vkResult && enableValidation )
				{
					// Error! Since the show must go on, try creating a Vulkan device instance without validation enabled...
					SE_LOG(WARNING, "Failed to create the Vulkan device instance with validation enabled, layer is not present");
					vkResult = createVkDevice(vkAllocationCallbacks, vkPhysicalDevice, vkDeviceQueueCreateInfo, false, enableDebugMarker, vkDevice);
				}
				// TODO(co) Error handling: Evaluate "vkResult"?
				graphicsQueueFamilyIndex = graphicsQueueIndex;
				presentQueueFamilyIndex = graphicsQueueIndex;	// TODO(co) Handle the case of the graphics queue doesn't support present

				// We're done, get us out of the loop
				graphicsQueueIndex = queueFamilyPropertyCount;
			}
		}
	}
	else
	{
		// Error!
		SE_LOG(CRITICAL, "Failed to get physical Vulkan device queue family properties")
	}

	// Done
	return vkDevice;
}
//-----------------------------------------------------------------------------
[[nodiscard]] VkCommandPool createVkCommandPool(const VkAllocationCallbacks *vkAllocationCallbacks, VkDevice vkDevice, uint32_t graphicsQueueFamilyIndex)
{
	VkCommandPool vkCommandPool = VK_NULL_HANDLE;

	// Create Vulkan command pool instance
	const VkCommandPoolCreateInfo vkCommandPoolCreateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,			// sType (VkStructureType)
		nullptr,											// pNext (const void*)
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,	// flags (VkCommandPoolCreateFlags)
		graphicsQueueFamilyIndex,							/// queueFamilyIndex (uint32_t)
	};
	const VkResult vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, vkAllocationCallbacks, &vkCommandPool);
	if( VK_SUCCESS != vkResult )
	{
		// Error!
		SE_LOG(CRITICAL, "Failed to create Vulkan command pool instance")
	}

	// Done
	return vkCommandPool;
}
//-----------------------------------------------------------------------------
[[nodiscard]] VkCommandBuffer createVkCommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool)
{
	VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;

	// Create Vulkan command buffer instance
	const VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	// sType (VkStructureType)
		nullptr,										// pNext (const void*)
		vkCommandPool,									// commandPool (VkCommandPool)
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,				// level (VkCommandBufferLevel)
		1												// commandBufferCount (uint32_t)
	};
	VkResult vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer);
	if( VK_SUCCESS != vkResult )
	{
		// Error!
		SE_LOG(CRITICAL, "Failed to create Vulkan command buffer instance")
	}

	// Done
	return vkCommandBuffer;
}
//-----------------------------------------------------------------------------
VulkanContext::VulkanContext(VKRenderer &renderer)
	: m_renderer(renderer)
	, m_vkPhysicalDevice(VK_NULL_HANDLE)
	, m_vkDevice(VK_NULL_HANDLE)
	, m_graphicsQueueFamilyIndex(~0u)
	, m_presentQueueFamilyIndex(~0u)
	, m_graphicsVkQueue(VK_NULL_HANDLE)
	, m_presentVkQueue(VK_NULL_HANDLE)
	, m_vkCommandPool(VK_NULL_HANDLE)
	, m_vkCommandBuffer(VK_NULL_HANDLE)
{
	const VulkanRuntimeLinking &vulkanRuntimeLinking = m_renderer.GetVulkanRuntimeLinking();

	// Get the physical Vulkan device this context should use
	bool enableDebugMarker = true;	// TODO(co) Make it possible to setup from the outside whether or not the "VK_EXT_debug_marker"-extension should be used (e.g. retail shipped games might not want to have this enabled)
	{
		detail::VkPhysicalDevices vkPhysicalDevices;
		detail::EnumeratePhysicalDevices(vulkanRuntimeLinking.GetVkInstance(), vkPhysicalDevices);
		if( !vkPhysicalDevices.empty() )
		{
			m_vkPhysicalDevice = selectPhysicalDevice(vkPhysicalDevices, m_renderer.GetVulkanRuntimeLinking().IsValidationEnabled(), enableDebugMarker);
		}
	}

	// Create the logical Vulkan device instance
	if( VK_NULL_HANDLE != m_vkPhysicalDevice )
	{
		m_vkDevice = createVkDevice(m_renderer.GetVkAllocationCallbacks(), m_vkPhysicalDevice, vulkanRuntimeLinking.IsValidationEnabled(), enableDebugMarker, m_graphicsQueueFamilyIndex, m_presentQueueFamilyIndex);
		if( VK_NULL_HANDLE != m_vkDevice )
		{
			// Load device based instance level Vulkan function pointers
			if( m_renderer.GetVulkanRuntimeLinking().LoadDeviceLevelVulkanEntryPoints(m_vkDevice) )
			{
				// Get the Vulkan device graphics queue that command buffers are submitted to
				vkGetDeviceQueue(m_vkDevice, m_graphicsQueueFamilyIndex, 0, &m_graphicsVkQueue);
				if( VK_NULL_HANDLE != m_graphicsVkQueue )
				{
					// Get the Vulkan device present queue
					vkGetDeviceQueue(m_vkDevice, m_presentQueueFamilyIndex, 0, &m_presentVkQueue);
					if( VK_NULL_HANDLE != m_presentVkQueue )
					{
						// Create Vulkan command pool instance
						m_vkCommandPool = createVkCommandPool(m_renderer.GetVkAllocationCallbacks(), m_vkDevice, m_graphicsQueueFamilyIndex);
						if( VK_NULL_HANDLE != m_vkCommandPool )
						{
							// Create Vulkan command buffer instance
							m_vkCommandBuffer = createVkCommandBuffer(m_vkDevice, m_vkCommandPool);
						}
						else
						{
							// Error!
							SE_LOG(CRITICAL, "Failed to create Vulkan command pool instance");
						}
					}
				}
				else
				{
					// Error!
					SE_LOG(CRITICAL, "Failed to get the Vulkan device graphics queue that command buffers are submitted to");
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
VulkanContext::~VulkanContext()
{
	if( VK_NULL_HANDLE != m_vkDevice )
	{
		if( VK_NULL_HANDLE != m_vkCommandPool )
		{
			if( VK_NULL_HANDLE != m_vkCommandBuffer )
			{
				vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &m_vkCommandBuffer);
			}
			vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, m_renderer.GetVkAllocationCallbacks());
		}
		vkDeviceWaitIdle(m_vkDevice);
		vkDestroyDevice(m_vkDevice, m_renderer.GetVkAllocationCallbacks());
	}
}
//-----------------------------------------------------------------------------
uint32_t VulkanContext::FindMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags vkMemoryPropertyFlags) const
{
	VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &vkPhysicalDeviceMemoryProperties);
	for( uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i )
	{
		if( (typeFilter & (1 << i)) && (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & vkMemoryPropertyFlags) == vkMemoryPropertyFlags )
		{
			return i;
		}
	}

	// Error!
	SE_LOG(CRITICAL, "Failed to find suitable Vulkan memory type");
	return ~0u;
}
//-----------------------------------------------------------------------------
VkCommandBuffer VulkanContext::CreateVkCommandBuffer() const
{
	return createVkCommandBuffer(m_vkDevice, m_vkCommandPool);
}
//-----------------------------------------------------------------------------
void VulkanContext::DestroyVkCommandBuffer(VkCommandBuffer vkCommandBuffer) const
{
	if( VK_NULL_HANDLE != m_vkCommandBuffer )
	{
		vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, 1, &vkCommandBuffer);
	}
}
//-----------------------------------------------------------------------------

SE_NAMESPACE_END
//=============================================================================

#endif