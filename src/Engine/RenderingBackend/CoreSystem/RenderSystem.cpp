#include "stdafx.h"
#include "RenderSystem.h"

//=============================================================================
SE_NAMESPACE_BEGIN

//-----------------------------------------------------------------------------
RenderSystem::RenderSystem(RenderConfig &config)
	: m_config(config)
{
#if SE_DIRECT3D11
#endif
#if SE_DIRECT3D12
#endif
#if SE_OPENGL
#endif
#if SE_VULKAN
	//if ( m_config.render == RenderingBackend::Vulkan )
	//	m_renderer = new VKRenderer();
#endif

	if ( !m_renderer || !m_renderer->Create(m_config) )
		return;

	std::string text = "Render Backend: ";
	switch ( m_config.render )
	{
	case RenderingBackend::Direct3D11: text += "Direct3D 11"; break;
	case RenderingBackend::Direct3D12: text += "Direct3D 12"; break;
	case RenderingBackend::OpenGL: text += "OpenGL"; break;
	case RenderingBackend::Vulkan: text += "Vulkan"; break;
	//Logs::Info(text);

	setValid(true);
}
//-----------------------------------------------------------------------------
RenderSystem::~RenderSystem()
{
	m_renderer->Destroy();
	SafeDelete(m_renderer);
}
//-----------------------------------------------------------------------------
bool RenderSystem::BeginFrame()
{
	if ( !m_renderer->BeginFrame() )
		return false;
	return true;
}
//-----------------------------------------------------------------------------
bool RenderSystem::EndFrame()
{
	if ( !m_renderer->EndFrame() )
		return false;
	return true;
}
//-----------------------------------------------------------------------------
void RenderSystem::Resize(unsigned width, unsigned height)
{
	m_renderer->Resize(width, height);
}
//-----------------------------------------------------------------------------

SE_NAMESPACE_END
//=============================================================================