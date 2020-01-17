#pragma once

#include "RenderingBackend/Resource/IResource.h"
#include "RenderingBackend/IRendererBackend.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IRenderPass : public IResource
{
public:
	inline virtual ~IRenderPass() override
	{
#if SE_STATISTICS
		// Update the statistics
		--GetRender().GetStatistics().currentNumberOfRenderPasses;
#endif
	}

protected:
	inline explicit IRenderPass(IRendererBackend *rhi SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) 
		: IResource(ResourceType::RENDER_PASS, rhi SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		// Update the statistics
		++rhi->GetStatistics().numberOfCreatedRenderPasses;
		++rhi->GetStatistics().currentNumberOfRenderPasses;
#endif
	}

	explicit IRenderPass(const IRenderPass&) = delete;
	IRenderPass& operator=(const IRenderPass&) = delete;
};

typedef SmartRefCount<IRenderPass> IRenderPassPtr;

SE_NAMESPACE_END
//=============================================================================