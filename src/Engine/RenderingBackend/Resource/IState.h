#pragma once

#include "RenderingBackend/Core/IResource.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IState : public IResource
{
public:
	virtual ~IState() override = default;

protected:
	IState(ResourceType resourceType, IRendererBackend *render SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IResource(resourceType, render SE_DEBUG_PASS_PARAMETER)
	{
	}

	explicit IState(const IState& source) = delete;
	IState& operator =(const IState& source) = delete;
};

typedef SmartRefCount<IState> IStatePtr;

SE_NAMESPACE_END
//=============================================================================