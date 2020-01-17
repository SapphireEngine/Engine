#pragma once

#include "Core/Template/Ref.h"
#include "Core/Debug/Assert.h"
#include "RenderingBackend/Core/ResourceType.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IRendererBackend;

// Abstract resource interface
class IResource : public RefCount<IResource>
{
public:
	virtual ~IResource() override = default;

	// Return the render implementation specific resource handle(e.g.native Direct3D texture pointer or OpenGL texture ID)
	[[nodiscard]] inline virtual void* GetInternalResourceHandle() const
	{
		return nullptr;
	}

	[[nodiscard]] inline ResourceType GetResourceType() const
	{
		return m_resourceType;
	}

#if SE_DEBUG
	[[nodiscard]] inline const char* GetDebugName() const
	{
		return m_debugName;
	}
#endif

	[[nodiscard]] inline IRendererBackend& GetRender() const
	{
		return *m_render;
	}

protected:
#if SE_DEBUG
	IResource(ResourceType resourceType, IRendererBackend *render, const char debugName[])
		: m_resourceType(resourceType)
		, m_render(render)
	{
		SE_ASSERT(strlen(debugName) < 256, "Resource debug name is not allowed to exceed 255 characters")
			strncpy(m_debugName, debugName, 256);
		m_debugName[255] = '\0';
	}
#else
	IResource(ResourceType resourceType, IRendererBackend *render)
		: m_resourceType(resourceType)
		, m_render(&render)
	{
	}
#endif

	explicit IResource(const IResource&) = delete;
	IResource& operator=(const IResource&) = delete;

private:
#if SE_DEBUG
	char m_debugName[256];	// Debug name for easier resource identification when debugging, contains terminating zero, first member variable by intent to see it at once during introspection (debug memory layout change is no problem here)
#endif
	ResourceType m_resourceType;
	IRendererBackend *m_render = nullptr;
};

typedef SmartRefCount<IResource> IResourcePtr;

SE_NAMESPACE_END
//=============================================================================