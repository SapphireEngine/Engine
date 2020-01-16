#include "stdafx.h"
#include "VKRenderer.h"
#include "VKCore.h"
#include "VKRuntimeLinking.h"
#include "Core/Memory/Memory.h"
#include "VKContext.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
VKRenderer::VKRenderer()
	: m_vkAllocationCallbacks{ GetSubsystem<Memory>().DefaultAlloc(), &detail::vkAllocationFunction, &detail::vkReallocationFunction, &detail::vkFreeFunction, nullptr, nullptr }
{
#if SE_DEBUG
	const bool enableValidation = true;
#else
	const bool enableValidation = false;
#endif

	// Is Vulkan available?
	m_vulkanRuntimeLinking = SE_NEW(VulkanRuntimeLinking)(*this, enableValidation);
	if( m_vulkanRuntimeLinking->IsVulkanAvaiable() )
	{
		// TODO(co) Add external Vulkan context support
		m_vulkanContext = SE_NEW(VulkanContext)(*this);

		// Is the Vulkan context initialized?
		if( m_vulkanContext->IsInitialized() )
		{
			// Initialize the capabilities
			initializeCapabilities();
		}
	}
}
//-----------------------------------------------------------------------------
bool VKRenderer::Create(RenderConfig &config, SE_NAMESPACE_WND::Window &window)
{
	return false;
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
SE_NAMESPACE_END
//=============================================================================