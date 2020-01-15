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
	{
	}

private:
	VKRenderer &m_renderer;
	bool m_validationEnabled;						// Validation enabled?
	void *m_vulkanSharedLibrary = nullptr;			// Vulkan shared library, can be a null pointer
	bool m_entryPointsRegistered = false;			// Entry points successfully registered?
	vk::Instance m_vkInstance = VK_NULL_HANDLE;		// Vulkan instance, stores all per-application states
	vk::DebugReportCallbackEXT m_vkDebugReportCallbackEXT = VK_NULL_HANDLE; // Vulkan debug report callback, can be a null handle
	bool m_instanceLevelFunctionsRegistered = false; // Instance level Vulkan function pointers registered?
	bool m_initialized = false;						// Already initialized?
};

SE_NAMESPACE_END
//=============================================================================