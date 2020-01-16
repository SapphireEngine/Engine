#pragma once

#include "Core/Memory/Memory.h"
#include "RenderingBackend/IRendererBackend.h"
#include "RenderingBackend/Vulkan/VKHeader.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class VulkanRuntimeLinking;
class VulkanContext;

class VKRenderer : public IRendererBackend
{
public:
	VKRenderer();

	bool Create(RenderConfig &config, SE_NAMESPACE_WND::Window &window) final;
	void Destroy() final;

	bool BeginFrame() final;
	bool EndFrame() final;

	void Resize(unsigned width, unsigned height) final;

	[[nodiscard]] const VkAllocationCallbacks* GetVkAllocationCallbacks() const
	{
#ifdef VK_USE_PLATFORM_WIN32_KHR
		return &m_vkAllocationCallbacks;
#else
		return nullptr;
#endif
	}

	[[nodiscard]] inline const VulkanRuntimeLinking& GetVulkanRuntimeLinking() const
	{
		return *m_vulkanRuntimeLinking;
	}

protected:
	virtual void selfDestruct() override
	{
		SE_DELETE(VKRenderer, this);
	}

private:
	VkAllocationCallbacks m_vkAllocationCallbacks;		// Vulkan allocation callbacks
	VulkanRuntimeLinking *m_vulkanRuntimeLinking = nullptr;	// Vulkan runtime linking instance, always valid
	VulkanContext *m_vulkanContext = nullptr; // Vulkan context instance, always valid
};

SE_NAMESPACE_END
//=============================================================================