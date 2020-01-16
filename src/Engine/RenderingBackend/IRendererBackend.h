#pragma once

#include "RenderingBackend/RenderCore.h"
#include "RenderingBackend/Capabilities.h"
#include "RenderingBackend/Statistics.h"
#include "Platform/Window/Window.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class SE_NO_VTABLE IRendererBackend
{
public:
	virtual ~IRendererBackend() = default;

	[[nodiscard]] inline const Capabilities& GetCapabilities() const
	{
		return m_capabilities;
	}

#if SE_STATISTICS
	[[nodiscard]] inline const Statistics& GetStatistics() const
	{
		return m_statistics;
	}
#endif

	virtual bool Create(RenderConfig &config, SE_NAMESPACE_WND::Window &window) = 0;
	virtual void Destroy() = 0;

	virtual bool BeginFrame() = 0;
	virtual bool EndFrame() = 0;

	virtual void Resize(unsigned width, unsigned height) = 0;

protected:
	Capabilities m_capabilities;
#ifdef SE_STATISTICS
	Statistics m_statistics;
#endif
};

SE_NAMESPACE_END
//=============================================================================