#pragma once

#include "RenderingBackend/Resource/ISwapChain.h"
#include "RenderingBackend/Vulkan/VKCore.h"

#if SE_VULKAN

//=============================================================================
SE_NAMESPACE_BEGIN

class VulkanContext;

class SwapChain final : public ISwapChain
{
public:
	[[nodiscard]] static VkFormat FindColorVkFormat(VkInstance vkInstance, const VulkanContext &vulkanContext);
	[[nodiscard]] static VkFormat FindDepthVkFormat(VkPhysicalDevice vkPhysicalDevice);
private:

};

SE_NAMESPACE_END
//=============================================================================

#endif