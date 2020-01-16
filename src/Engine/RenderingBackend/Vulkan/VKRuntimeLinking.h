#pragma once

#include "RenderingBackend/Vulkan/VKRenderer.h"

// Vulkan runtime linking for creating and managing the Vulkan instance ("VkInstance")

//=============================================================================
SE_NAMESPACE_BEGIN

class VulkanRuntimeLinking final
{
public:
	VulkanRuntimeLinking(VKRenderer &renderer, bool enableValidation)
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

	~VulkanRuntimeLinking()
	{
		// Destroy the Vulkan debug report callback
		if( VK_NULL_HANDLE != m_vkDebugReportCallbackEXT )
		{
			vkDestroyDebugReportCallbackEXT(m_vkInstance, m_vkDebugReportCallbackEXT, m_renderer.GetVkAllocationCallbacks());
		}

		// Destroy the Vulkan instance
		if( VK_NULL_HANDLE != m_vkInstance )
		{
			vkDestroyInstance(m_vkInstance, m_renderer.GetVkAllocationCallbacks());
		}

		// Destroy the shared library instances
#ifdef _WIN32
		if( nullptr != m_vulkanSharedLibrary )
		{
			::FreeLibrary(static_cast<HMODULE>(m_vulkanSharedLibrary));
		}
#elif defined LINUX
		if( nullptr != mVulkanSharedLibrary )
		{
			::dlclose(mVulkanSharedLibrary);
		}
#else
#error "Unsupported platform"
#endif
	}

	// Return whether or not validation is enabled
	[[nodiscard]] bool IsValidationEnabled() const
	{
		return m_validationEnabled;
	}

	[[nodiscard]] VkInstance GetVkInstance() const
	{
		return m_vkInstance;
	}

	[[nodiscard]] bool IsVulkanAvaiable();

	[[nodiscard]] bool LoadDeviceLevelVulkanEntryPoints(VkDevice vkDevice) const;

private:
	[[nodiscard]] bool loadSharedLibraries();
	[[nodiscard]] bool loadGlobalLevelVulkanEntryPoints() const;
	[[nodiscard]] VkResult createVulkanInstance(bool enableValidation);
	[[nodiscard]] bool loadInstanceLevelVulkanEntryPoints() const;
	void setupDebugCallback();

	VKRenderer &m_renderer;
	bool m_validationEnabled; // Validation enabled?
	void *m_vulkanSharedLibrary; // Vulkan shared library, can be a null pointer
	bool m_entryPointsRegistered; // Entry points successfully registered?
	VkInstance m_vkInstance; // Vulkan instance, stores all per-application states
	VkDebugReportCallbackEXT m_vkDebugReportCallbackEXT; // Vulkan debug report callback, can be a null handle
	bool m_instanceLevelFunctionsRegistered; // Instance level Vulkan function pointers registered?
	bool m_initialized; // Already initialized?
};

SE_NAMESPACE_END
//=============================================================================