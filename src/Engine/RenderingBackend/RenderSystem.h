#pragma once

#include "RenderingBackend/RenderCore.h"
#include "Platform/Window/Window.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IRendererBackend;

class RenderSystem
{
public:
	RenderSystem(RenderConfig &config, SE_NAMESPACE_WND::Window &window);
	~RenderSystem();

	bool BeginFrame();
	bool EndFrame();

	void Resize(unsigned width, unsigned height);

private:
	RenderConfig &m_config;
	IRendererBackend *m_renderer = nullptr;
};

SE_NAMESPACE_END
//=============================================================================