#pragma once

#include "RenderingBackend/IRendererBackend.h"
#include "RenderingBackend/CoreHeader/Vulkan.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class VulkanRuntimeLinking;

class VKRenderer : public IRendererBackend
{
public:
	VKRenderer();

	bool Create(RenderConfig &config, SE_NAMESPACE_WND::Window &window) final;
	void Destroy() final;

	bool BeginFrame() final;
	bool EndFrame() final;

	void Resize(unsigned width, unsigned height) final;

private:
	vk::AllocationCallbacks m_VkAllocationCallbacks;		// Vulkan allocation callbacks
	VulkanRuntimeLinking *m_vulkanRuntimeLinking = nullptr;	// Vulkan runtime linking instance, always valid
};

SE_NAMESPACE_END
//=============================================================================