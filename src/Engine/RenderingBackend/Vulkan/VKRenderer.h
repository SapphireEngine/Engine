#pragma once

#include "RenderingBackend/IRendererBackend.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class VKRenderer : public IRendererBackend
{
public:
	bool Create(RenderConfig &config, SE_NAMESPACE_WND::Window &window) final;
	void Destroy() final;

	bool BeginFrame() final;
	bool EndFrame() final;

	void Resize(unsigned width, unsigned height) final;
};

SE_NAMESPACE_END
//=============================================================================