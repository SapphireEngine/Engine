#include "stdafx.h"
#include "VKRuntimeLinking.h"
#include "Core/Debug/Log.h"
#include "VKRenderer.h"
#include "VKCore.h"

#if SE_VULKAN

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
VulkanRuntimeLinking::VulkanRuntimeLinking(VKRenderer &renderer, bool enableValidation)
	: m_renderer(renderer)
	, m_validationEnabled(enableValidation)
	, m_vulkanSharedLibrary(nullptr)
	, m_entryPointsRegistered(false)
	, m_vkInstance(VK_NULL_HANDLE)
	, m_vkDebugReportCallbackEXT(VK_NULL_HANDLE)
	, m_instanceLevelFunctionsRegistered(false)
	, m_initialized(false)
{
}
//-----------------------------------------------------------------------------
VulkanRuntimeLinking::~VulkanRuntimeLinking()
{
	// Destroy the Vulkan debug report callback
	if ( VK_NULL_HANDLE != m_vkDebugReportCallbackEXT )
	{
		vkDestroyDebugReportCallbackEXT(m_vkInstance, m_vkDebugReportCallbackEXT, m_renderer.GetVkAllocationCallbacks());
	}

	// Destroy the Vulkan instance
	if ( VK_NULL_HANDLE != m_vkInstance )
	{
		vkDestroyInstance(m_vkInstance, m_renderer.GetVkAllocationCallbacks());
	}

	// Destroy the shared library instances
#if SE_PLATFORM_WINDOWS
	if ( nullptr != m_vulkanSharedLibrary )
	{
		::FreeLibrary(static_cast<HMODULE>(m_vulkanSharedLibrary));
	}
#elif SE_PLATFORM_LINUX
	if ( nullptr != mVulkanSharedLibrary )
	{
		::dlclose(mVulkanSharedLibrary);
	}
#else
#	error "Unsupported platform"
#endif
}
//-----------------------------------------------------------------------------
bool VulkanRuntimeLinking::IsVulkanAvaiable()
{
	// Already initialized?
	if( !m_initialized )
	{
		// We're now initialized
		m_initialized = true;

		// Load the shared libraries
		if( loadSharedLibraries() )
		{
			// Load the global level Vulkan function entry points
			m_entryPointsRegistered = loadGlobalLevelVulkanEntryPoints();
			if( m_entryPointsRegistered )
			{
				// Create the Vulkan instance
				const VkResult result = createVulkanInstance(m_validationEnabled);
				if( VK_SUCCESS == result )
				{
					// Load instance based instance level Vulkan function pointers
					m_instanceLevelFunctionsRegistered = loadInstanceLevelVulkanEntryPoints();

					// Setup debug callback
					if( m_instanceLevelFunctionsRegistered && m_validationEnabled )
					{
						setupDebugCallback();
					}
				}
				else
				{
					// Error!
					SE_LOG(CRITICAL, "Failed to create the Vulkan instance")
				}
			}
		}
	}

	// Entry points successfully registered?
	return (m_entryPointsRegistered && (VK_NULL_HANDLE != m_vkInstance) && m_instanceLevelFunctionsRegistered);
}
//-----------------------------------------------------------------------------
bool VulkanRuntimeLinking::LoadDeviceLevelVulkanEntryPoints(VkDevice vkDevice) const
{
	bool result = true;	// Success by default

#define IMPORT_FUNC(funcName) \
	if (result) \
	{ \
		funcName = reinterpret_cast<PFN_##funcName>(vkGetDeviceProcAddr(vkDevice, #funcName)); \
		if (nullptr == funcName) \
		{ \
			SE_LOG(CRITICAL, "Failed to load instance based Vulkan function pointer \"%s\"", #funcName) \
			result = false;	 \
		} \
	}

	// Load the Vulkan device level function entry points
	IMPORT_FUNC(vkDestroyDevice)
	IMPORT_FUNC(vkCreateShaderModule)
	IMPORT_FUNC(vkDestroyShaderModule)
	IMPORT_FUNC(vkCreateBuffer)
	IMPORT_FUNC(vkDestroyBuffer)
	IMPORT_FUNC(vkMapMemory)
	IMPORT_FUNC(vkUnmapMemory)
	IMPORT_FUNC(vkCreateBufferView)
	IMPORT_FUNC(vkDestroyBufferView)
	IMPORT_FUNC(vkAllocateMemory)
	IMPORT_FUNC(vkFreeMemory)
	IMPORT_FUNC(vkGetBufferMemoryRequirements)
	IMPORT_FUNC(vkBindBufferMemory)
	IMPORT_FUNC(vkCreateRenderPass)
	IMPORT_FUNC(vkDestroyRenderPass)
	IMPORT_FUNC(vkCreateImage)
	IMPORT_FUNC(vkDestroyImage)
	IMPORT_FUNC(vkGetImageSubresourceLayout)
	IMPORT_FUNC(vkGetImageMemoryRequirements)
	IMPORT_FUNC(vkBindImageMemory)
	IMPORT_FUNC(vkCreateImageView)
	IMPORT_FUNC(vkDestroyImageView)
	IMPORT_FUNC(vkCreateSampler)
	IMPORT_FUNC(vkDestroySampler)
	IMPORT_FUNC(vkCreateSemaphore)
	IMPORT_FUNC(vkDestroySemaphore)
	IMPORT_FUNC(vkCreateFence)
	IMPORT_FUNC(vkDestroyFence)
	IMPORT_FUNC(vkWaitForFences)
	IMPORT_FUNC(vkCreateCommandPool)
	IMPORT_FUNC(vkDestroyCommandPool)
	IMPORT_FUNC(vkAllocateCommandBuffers)
	IMPORT_FUNC(vkFreeCommandBuffers)
	IMPORT_FUNC(vkBeginCommandBuffer)
	IMPORT_FUNC(vkEndCommandBuffer)
	IMPORT_FUNC(vkGetDeviceQueue)
	IMPORT_FUNC(vkQueueSubmit)
	IMPORT_FUNC(vkQueueWaitIdle)
	IMPORT_FUNC(vkDeviceWaitIdle)
	IMPORT_FUNC(vkCreateFramebuffer)
	IMPORT_FUNC(vkDestroyFramebuffer)
	IMPORT_FUNC(vkCreatePipelineCache)
	IMPORT_FUNC(vkDestroyPipelineCache)
	IMPORT_FUNC(vkCreatePipelineLayout)
	IMPORT_FUNC(vkDestroyPipelineLayout)
	IMPORT_FUNC(vkCreateGraphicsPipelines)
	IMPORT_FUNC(vkCreateComputePipelines)
	IMPORT_FUNC(vkDestroyPipeline)
	IMPORT_FUNC(vkCreateDescriptorPool)
	IMPORT_FUNC(vkDestroyDescriptorPool)
	IMPORT_FUNC(vkCreateDescriptorSetLayout)
	IMPORT_FUNC(vkDestroyDescriptorSetLayout)
	IMPORT_FUNC(vkAllocateDescriptorSets)
	IMPORT_FUNC(vkFreeDescriptorSets)
	IMPORT_FUNC(vkUpdateDescriptorSets)
	IMPORT_FUNC(vkCreateQueryPool)
	IMPORT_FUNC(vkDestroyQueryPool)
	IMPORT_FUNC(vkGetQueryPoolResults)
	IMPORT_FUNC(vkCmdBeginQuery)
	IMPORT_FUNC(vkCmdEndQuery)
	IMPORT_FUNC(vkCmdResetQueryPool)
	IMPORT_FUNC(vkCmdWriteTimestamp)
	IMPORT_FUNC(vkCmdCopyQueryPoolResults)
	IMPORT_FUNC(vkCmdPipelineBarrier)
	IMPORT_FUNC(vkCmdBeginRenderPass)
	IMPORT_FUNC(vkCmdEndRenderPass)
	IMPORT_FUNC(vkCmdExecuteCommands)
	IMPORT_FUNC(vkCmdCopyImage)
	IMPORT_FUNC(vkCmdBlitImage)
	IMPORT_FUNC(vkCmdCopyBufferToImage)
	IMPORT_FUNC(vkCmdClearAttachments)
	IMPORT_FUNC(vkCmdCopyBuffer)
	IMPORT_FUNC(vkCmdBindDescriptorSets)
	IMPORT_FUNC(vkCmdBindPipeline)
	IMPORT_FUNC(vkCmdSetViewport)
	IMPORT_FUNC(vkCmdSetScissor)
	IMPORT_FUNC(vkCmdSetLineWidth)
	IMPORT_FUNC(vkCmdSetDepthBias)
	IMPORT_FUNC(vkCmdPushConstants)
	IMPORT_FUNC(vkCmdBindIndexBuffer)
	IMPORT_FUNC(vkCmdBindVertexBuffers)
	IMPORT_FUNC(vkCmdDraw)
	IMPORT_FUNC(vkCmdDrawIndexed)
	IMPORT_FUNC(vkCmdDrawIndirect)
	IMPORT_FUNC(vkCmdDrawIndexedIndirect)
	IMPORT_FUNC(vkCmdDispatch)
	IMPORT_FUNC(vkCmdClearColorImage)
	IMPORT_FUNC(vkCmdClearDepthStencilImage)
	// "VK_KHR_swapchain"-extension
	IMPORT_FUNC(vkCreateSwapchainKHR)
	IMPORT_FUNC(vkDestroySwapchainKHR)
	IMPORT_FUNC(vkGetSwapchainImagesKHR)
	IMPORT_FUNC(vkAcquireNextImageKHR)
	IMPORT_FUNC(vkQueuePresentKHR)

#undef IMPORT_FUNC
	return result;
}
//-----------------------------------------------------------------------------
bool VulkanRuntimeLinking::loadSharedLibraries()
{
#if SE_PLATFORM_WINDOWS
	m_vulkanSharedLibrary = ::LoadLibraryExA("vulkan-1.dll", nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
	if( nullptr == m_vulkanSharedLibrary )
		SE_LOG(CRITICAL, "Failed to load in the shared Vulkan library \"vulkan-1.dll\"")
#elif SE_PLATFORM_LINUX
	mVulkanSharedLibrary = ::dlopen("libvulkan.so", RTLD_NOW);
	if( nullptr == mVulkanSharedLibrary )
		SE_LOG(mVulkanRhi.getContext(), CRITICAL, "Failed to load in the shared Vulkan library \"libvulkan-1.so\"")
#else
#	error "Unsupported platform"
#endif
	return (nullptr != m_vulkanSharedLibrary);
}
//-----------------------------------------------------------------------------
bool VulkanRuntimeLinking::loadGlobalLevelVulkanEntryPoints() const
{
	bool result = true;	// Success by default

	// Define a helper macro
#if SE_PLATFORM_WINDOWS

#	define IMPORT_FUNC(funcName) \
		if (result)  \
		{ \
			void *symbol = ::GetProcAddress(static_cast<HMODULE>(m_vulkanSharedLibrary), #funcName); \
			if (nullptr != symbol) \
			{ \
				*(reinterpret_cast<void**>(&(funcName))) = symbol; \
			} \
			else \
			{ \
				wchar_t moduleFilename[MAX_PATH]; \
				moduleFilename[0] = '\0'; \
				::GetModuleFileNameW(static_cast<HMODULE>(m_vulkanSharedLibrary), moduleFilename, MAX_PATH); \
				SE_LOG(CRITICAL, "Failed to locate the entry point \"%s\" within the shared Vulkan library \"%s\"", #funcName, moduleFilename) \
				result = false; \
			} \
		}
#elif SE_PLATFORM_ANDROID
#	define IMPORT_FUNC(funcName) \
		if (result) \
		{ \
			void *symbol = ::dlsym(mVulkanSharedLibrary, #funcName); \
			if (nullptr != symbol) \
			{ \
				*(reinterpret_cast<void**>(&(funcName))) = symbol; \
			} \
			else \
			{ \
				const char* libraryName = "unknown"; \
				SE_LOG(CRITICAL, "Failed to locate the Vulkan entry point \"%s\" within the Vulkan shared library \"%s\"", #funcName, libraryName) \
				result = false;	 \
			} \
		}
#elif SE_PLATFORM_LINUX
#	define IMPORT_FUNC(funcName) \
		if (result) \
		{ \
			void *symbol = ::dlsym(mVulkanSharedLibrary, #funcName); \
			if (nullptr != symbol) \
			{ \
				*(reinterpret_cast<void**>(&(funcName))) = symbol; \
			} \
			else \
			{ \
				link_map *linkMap = nullptr; \
				const char *libraryName = "unknown"; \
				if (dlinfo(mVulkanSharedLibrary, RTLD_DI_LINKMAP, &linkMap)) \
				{ \
					libraryName = linkMap->l_name; \
				} \
				SE_LOG(CRITICAL, "Failed to locate the entry point \"%s\" within the shared Vulkan library \"%s\"", #funcName, libraryName) \
				result = false;	 \
			} \
		}
#else
#	error "Unsupported platform"
#endif

// Load the Vulkan global level function entry points
	IMPORT_FUNC(vkGetInstanceProcAddr);
	IMPORT_FUNC(vkGetDeviceProcAddr);
	IMPORT_FUNC(vkEnumerateInstanceExtensionProperties);
	IMPORT_FUNC(vkEnumerateInstanceLayerProperties);
	IMPORT_FUNC(vkCreateInstance);
#undef IMPORT_FUNC

return result;
}
//-----------------------------------------------------------------------------
VkResult VulkanRuntimeLinking::createVulkanInstance(bool enableValidation)
{
	// Enable surface extensions depending on OS
	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
#ifdef VK_USE_PLATFORM_WIN32_KHR
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_ANDROID_KHR
	enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_XLIB_KHR
	enabledExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
	enabledExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_XCB_KHR
	enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#else
#	error "Unsupported platform"
#endif
	if( enableValidation )
	{
		enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	// Ensure the extensions we need are supported
	{ 
		uint32_t propertyCount = 0;
		if( (vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr) != VK_SUCCESS) || (0 == propertyCount) )
		{
			SE_LOG(CRITICAL, "Failed to enumerate Vulkan instance extension properties");
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
		detail::VkExtensionPropertiesVector vkExtensionPropertiesVector(propertyCount);
		if( vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, &vkExtensionPropertiesVector[0]) != VK_SUCCESS )
		{
			SE_LOG(CRITICAL, "Failed to enumerate Vulkan instance extension properties");
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
		for( const char *enabledExtension : enabledExtensions )
		{
			if( !detail::IsExtensionAvailable(enabledExtension, vkExtensionPropertiesVector) )
			{
				SE_LOG(CRITICAL, "Couldn't find Vulkan instance extension named \"%s\"", enabledExtension);
				return VK_ERROR_EXTENSION_NOT_PRESENT;
			}
		}
	}

	// TODO(co) Make it possible for the user to provide application related information?
	static constexpr VkApplicationInfo vkApplicationInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,	// sType (VkStructureType)
		nullptr,							// pNext (const void*)
		"Sapphire Application",				// pApplicationName (const char*)
		VK_MAKE_VERSION(0, 0, 0),			// applicationVersion (uint32_t)
		"Sapphire",							// pEngineName (const char*)
		VK_MAKE_VERSION(0, 0, 0),			// engineVersion (uint32_t)
		VK_API_VERSION_1_0					// apiVersion (uint32_t)
	};

	const VkInstanceCreateInfo vkInstanceCreateInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,							// sType (VkStructureType)
		nullptr,														// pNext (const void*)
		0,																// flags (VkInstanceCreateFlags)
		&vkApplicationInfo,												// pApplicationInfo (const VkApplicationInfo*)
		enableValidation ? detail::NUMBER_OF_VALIDATION_LAYERS : 0,		// enabledLayerCount (uint32_t)
		enableValidation ? detail::VALIDATION_LAYER_NAMES : nullptr,	// ppEnabledLayerNames (const char* const*)
		static_cast<uint32_t>(enabledExtensions.size()),				// enabledExtensionCount (uint32_t)
		enabledExtensions.data()										// ppEnabledExtensionNames (const char* const*)
	};
	VkResult vkResult = vkCreateInstance(&vkInstanceCreateInfo, m_renderer.GetVkAllocationCallbacks(), &m_vkInstance);
	if( VK_ERROR_LAYER_NOT_PRESENT == vkResult && enableValidation )
	{
		// Error! Since the show must go on, try creating a Vulkan instance without validation enabled...
		SE_LOG(WARNING, "Failed to create the Vulkan instance with validation enabled, layer is not present. Install e.g. the LunarG Vulkan SDK and see e.g. https://vulkan.lunarg.com/doc/view/1.0.51.0/windows/layers.html .");
		m_validationEnabled = false;
		vkResult = createVulkanInstance(m_validationEnabled);
	}

	return vkResult;
}
//-----------------------------------------------------------------------------
bool VulkanRuntimeLinking::loadInstanceLevelVulkanEntryPoints() const
{
	bool result = true;	// Success by default

#define IMPORT_FUNC(funcName) \
	if (result) \
	{ \
		funcName = reinterpret_cast<PFN_##funcName>(vkGetInstanceProcAddr(m_vkInstance, #funcName)); \
		if (nullptr == funcName) \
		{ \
			SE_LOG(CRITICAL, "Failed to load instance based Vulkan function pointer \"%s\"", #funcName) \
			result = false; \
		} \
	}

	// Load the Vulkan instance level function entry points
	IMPORT_FUNC(vkDestroyInstance);
	IMPORT_FUNC(vkEnumeratePhysicalDevices);
	IMPORT_FUNC(vkEnumerateDeviceLayerProperties);
	IMPORT_FUNC(vkEnumerateDeviceExtensionProperties);
	IMPORT_FUNC(vkGetPhysicalDeviceQueueFamilyProperties);
	IMPORT_FUNC(vkGetPhysicalDeviceFeatures);
	IMPORT_FUNC(vkGetPhysicalDeviceFormatProperties);
	IMPORT_FUNC(vkGetPhysicalDeviceMemoryProperties);
	IMPORT_FUNC(vkGetPhysicalDeviceProperties);
	IMPORT_FUNC(vkCreateDevice);
	if( m_validationEnabled )
	{
		// "VK_EXT_debug_report"-extension
		IMPORT_FUNC(vkCreateDebugReportCallbackEXT);
		IMPORT_FUNC(vkDestroyDebugReportCallbackEXT);
	}
	// "VK_KHR_surface"-extension
	IMPORT_FUNC(vkDestroySurfaceKHR)
	IMPORT_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR)
	IMPORT_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR)
	IMPORT_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
	IMPORT_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR)
#ifdef VK_USE_PLATFORM_WIN32_KHR
	// "VK_KHR_win32_surface"-extension
	IMPORT_FUNC(vkCreateWin32SurfaceKHR)
#elif defined VK_USE_PLATFORM_ANDROID_KHR
	// "VK_KHR_android_surface"-extension
	IMPORT_FUNC(vkCreateAndroidSurfaceKHR)
#elif defined VK_USE_PLATFORM_XLIB_KHR
	// "VK_KHR_xlib_surface"-extension
	IMPORT_FUNC(vkCreateXlibSurfaceKHR)
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
	// "VK_KHR_wayland_surface"-extension
	IMPORT_FUNC(vkCreateWaylandSurfaceKHR)
#elif defined VK_USE_PLATFORM_XCB_KHR
	// "VK_KHR_xcb_surface"-extension
	IMPORT_FUNC(vkCreateXcbSurfaceKHR)
#else
#	error "Unsupported platform"
#endif
#undef IMPORT_FUNC

	return result;
}
//-----------------------------------------------------------------------------
void VulkanRuntimeLinking::setupDebugCallback()
{
	// Sanity check
	SE_ASSERT(m_validationEnabled, "Do only call this Vulkan method if validation is enabled");

	// The report flags determine what type of messages for the layers will be displayed
	// -> Use "VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT" to get everything, quite verbose
	static constexpr VkDebugReportFlagsEXT vkDebugReportFlagsEXT = (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);

	// Setup debug callback
	const VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT =
	{
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,	// sType (VkStructureType)
		nullptr,													// pNext (const void*)
		vkDebugReportFlagsEXT,										// flags (VkDebugReportFlagsEXT)
		detail::DebugReportCallback,								// pfnCallback (PFN_vkDebugReportCallbackEXT)
		nullptr														// pUserData (void*)
	};

	if( vkCreateDebugReportCallbackEXT(m_vkInstance, &vkDebugReportCallbackCreateInfoEXT, m_renderer.GetVkAllocationCallbacks(), &m_vkDebugReportCallbackEXT) != VK_SUCCESS )
	{
		SE_LOG(WARNING, "Failed to create the Vulkan debug report callback");
	}
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================

#endif