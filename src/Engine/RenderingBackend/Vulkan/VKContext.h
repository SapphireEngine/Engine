#pragma once

#include "RenderingBackend/Vulkan/VKRenderer.h"

#if SE_VULKAN

//=============================================================================
SE_NAMESPACE_BEGIN

class VulkanContext final
{
public:
	VulkanContext(VKRenderer &renderer);
	~VulkanContext();

	// Return whether or not the content is initialized
	[[nodiscard]] bool IsInitialized() const
	{
		return (VK_NULL_HANDLE != m_vkCommandBuffer);
	}

	// Return the Vulkan physical device this context is using
	[[nodiscard]] VkPhysicalDevice GetVkPhysicalDevice() const
	{
		return m_vkPhysicalDevice;
	}

	// Return the Vulkan device this context is using
	[[nodiscard]] VkDevice GetVkDevice() const
	{
		return m_vkDevice;
	}

	//Return the used graphics queue family index
	[[nodiscard]] uint32_t GetGraphicsQueueFamilyIndex() const
	{
		return m_graphicsQueueFamilyIndex;
	}

	// Return the used present queue family index
	[[nodiscard]] uint32_t GetPresentQueueFamilyIndex() const
	{
		return m_presentQueueFamilyIndex;
	}

	// Return the handle to the Vulkan device graphics queue that command buffers are submitted to
	[[nodiscard]] inline VkQueue GetGraphicsVkQueue() const
	{
		return m_graphicsVkQueue;
	}

	// Return the handle to the Vulkan device present queue
	[[nodiscard]] VkQueue GetPresentVkQueue() const
	{
		return m_presentVkQueue;
	}

	// Return the used Vulkan command buffer pool instance
	[[nodiscard]] VkCommandPool GetVkCommandPool() const
	{
		return m_vkCommandPool;
	}

	// Return the Vulkan command buffer instance
	[[nodiscard]] VkCommandBuffer GetVkCommandBuffer() const
	{
		return m_vkCommandBuffer;
	}

	// TODO(co) Trivial implementation to have something to start with. Need to use more clever memory management and stating buffers later on.
	[[nodiscard]] uint32_t FindMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags vkMemoryPropertyFlags) const;

	[[nodiscard]] VkCommandBuffer CreateVkCommandBuffer() const;	
	void DestroyVkCommandBuffer(VkCommandBuffer vkCommandBuffer) const;

private:
	VKRenderer &m_renderer;
	VkPhysicalDevice m_vkPhysicalDevice; // Vulkan physical device this context is using
	VkDevice m_vkDevice; // Vulkan device instance this context is using (equivalent of a OpenGL context or Direct3D 11 device)
	uint32_t m_graphicsQueueFamilyIndex; // Graphics queue family index, ~0u if invalid
	uint32_t m_presentQueueFamilyIndex; //< Present queue family index, ~0u if invalid
	VkQueue m_graphicsVkQueue; // Handle to the Vulkan device graphics queue that command buffers are submitted to
	VkQueue m_presentVkQueue; // Handle to the Vulkan device present queue
	VkCommandPool m_vkCommandPool; // Vulkan command buffer pool instance
	VkCommandBuffer m_vkCommandBuffer; // Vulkan command buffer instance
};

SE_NAMESPACE_END
//=============================================================================

#endif