#pragma once

#include "RenderingBackend/Vulkan/VKHeader.h"

#if SE_VULKAN

// Vulkan runtime linking for creating and managing the Vulkan instance ("VkInstance")

//=============================================================================
SE_NAMESPACE_BEGIN

class VKRenderer;

class VulkanRuntimeLinking final
{
public:
	VulkanRuntimeLinking(VKRenderer &renderer, bool enableValidation);
	~VulkanRuntimeLinking();

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

#endif