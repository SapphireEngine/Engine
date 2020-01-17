#pragma once

#include "RenderingBackend/Resource/IRenderPass.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IRenderTarget : public IResource
{
public:
	virtual ~IRenderTarget() override
	{
		m_renderPass.ReleaseReference();
	}

	[[nodiscard]] inline IRenderPass& GetRenderPass() const
	{
		return m_renderPass;
	}

	virtual void GetWidthAndHeight(uint32_t &width, uint32_t &height) const = 0;

protected:
	IRenderTarget(ResourceType resourceType, IRenderPass &renderPass SE_DEBUG_NAME_PARAMETER_NO_DEFAULT)
		: IResource(resourceType, &renderPass.GetRender() SE_DEBUG_PASS_PARAMETER)
		, m_renderPass(renderPass)
	{
		m_renderPass.AddReference();
	}

	explicit IRenderTarget(const IRenderTarget&) = delete;
	IRenderTarget& operator=(const IRenderTarget&) = delete;
private:
	IRenderPass &m_renderPass;
};

typedef SmartRefCount<IRenderTarget> IRenderTargetPtr;

SE_NAMESPACE_END
//=============================================================================