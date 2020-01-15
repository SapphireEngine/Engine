#include "stdafx.h"
#include "VKRenderer.h"
#include "VKCore.h"
#include "VKRuntimeLinking.h"
#include "Core/Memory/Memory.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
VKRenderer::VKRenderer()
	: m_VkAllocationCallbacks(GetSubsystem<Memory>().DefaultAlloc(), &detail::vkAllocationFunction, &detail::vkReallocationFunction, &detail::vkFreeFunction, nullptr, nullptr )
{
#if SE_DEBUG
	const bool enableValidation = true;
#else
	const bool enableValidation = false;
#endif

	m_vulkanRuntimeLinking = SE_NEW(VulkanRuntimeLinking)(*this, enableValidation);
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