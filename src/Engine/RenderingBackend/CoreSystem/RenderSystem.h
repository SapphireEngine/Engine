#pragma once

#include "Core/Object/Subsystem.h"
#include "RenderingBackend/CoreSystem/IRendererBackend.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class RenderSystem : public Subsystem<RenderSystem>
{
public:
	RenderSystem(RenderConfig &config);
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