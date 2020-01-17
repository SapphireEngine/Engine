#include "stdafx.h"
#include "VKRenderer.h"
#include "VKCore.h"
#include "VKRuntimeLinking.h"
#include "Core/Memory/Memory.h"
#include "VKContext.h"

#if SE_VULKAN

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
VKRenderer::VKRenderer()
	: m_vkAllocationCallbacks{ GetSubsystem<Memory>().DefaultAlloc(), &detail::VKAllocationFunction, &detail::VKReallocationFunction, &detail::VKFreeFunction, nullptr, nullptr }
{
}
//-----------------------------------------------------------------------------
bool VKRenderer::Create(RenderConfig &config, SE_NAMESPACE_WND::Window &window)
{
#if SE_DEBUG
	const bool enableValidation = true;
#else
	const bool enableValidation = false;
#endif

	// Is Vulkan available?
	m_vulkanRuntimeLinking = SE_NEW(VulkanRuntimeLinking)(*this, enableValidation);
	if ( m_vulkanRuntimeLinking->IsVulkanAvaiable() )
	{
		// TODO(co) Add external Vulkan context support
		m_vulkanContext = SE_NEW(VulkanContext)(*this);

		// Is the Vulkan context initialized?
		if ( m_vulkanContext->IsInitialized() )
		{
			// Initialize the capabilities
			initializeCapabilities();
		}
	}

	return true;
}
//-----------------------------------------------------------------------------
void VKRenderer::Destroy()
{
}
//-----------------------------------------------------------------------------
bool VKRenderer::BeginFrame()
{
	return false;
}
//-----------------------------------------------------------------------------
bool VKRenderer::EndFrame()
{
	return false;
}
//-----------------------------------------------------------------------------
void VKRenderer::Resize(unsigned width, unsigned height)
{
}
//-----------------------------------------------------------------------------
void VKRenderer::selfDestruct()
{
	SE_DELETE(VKRenderer, this);
}
//-----------------------------------------------------------------------------
void VKRenderer::initializeCapabilities()
{
	{ // Get device name
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_vulkanContext->GetVkPhysicalDevice(), &vkPhysicalDeviceProperties);
		const size_t numberOfCharacters = Countof(m_capabilities.deviceName) - 1;
		strncpy(m_capabilities.deviceName, vkPhysicalDeviceProperties.deviceName, numberOfCharacters);
		m_capabilities.deviceName[numberOfCharacters] = '\0';
	}


}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================

#endif