#pragma once

#include "RenderingBackend/Resource/IResource.h"
#include "RenderingBackend/CoreSystem/IRendererBackend.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IResourceGroup;

// Abstract root signature ("pipeline layout" in Vulkan terminology) interface
//note - Overview of the binding models of explicit APIs: "Choosing a binding model" - https://github.com/gpuweb/gpuweb/issues/19
class IRootSignature : public IResource
{
public:
	inline virtual ~IRootSignature() override
	{
#if SE_STATISTICS
		--GetRender().getStatistics().currentNumberOfRootSignatures;
#endif
	}

	/**
	*  @brief
	*    Create a resource group instance
	*
	*  @param[in] rootParameterIndex
	*    The root parameter index number for binding
	*  @param[in] numberOfResources
	*    Number of resources, having no resources is invalid
	*  @param[in] resources
	*    At least "numberOfResources" resource pointers, must be valid, the resource group will keep a reference to the resources
	*  @param[in] samplerStates
	*    If not a null pointer at least "numberOfResources" sampler state pointers, must be valid if there's at least one texture resource, the resource group will keep a reference to the sampler states
	*
	*  @return
	*    The created resource group instance, a null pointer on error. Release the returned instance if you no longer need it.
	*/
	[[nodiscard]] virtual IResourceGroup* CreateResourceGroup(uint32_t rootParameterIndex, uint32_t numberOfResources, IResource** resources, ISamplerState** samplerStates = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	// Protected methods
protected:
	inline explicit IRootSignature(IRendererBackend &render SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IResource(ResourceType::ROOT_SIGNATURE, render SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		// Update the statistics
		++render.getStatistics().numberOfCreatedRootSignatures;
		++render.getStatistics().currentNumberOfRootSignatures;
#endif
	}

	explicit IRootSignature(const IRootSignature&) = delete;
	IRootSignature& operator=(const IRootSignature&) = delete;
};

typedef SmartRefCount<IRootSignature> IRootSignaturePtr;

/**
*  @brief
*    Abstract resource group interface
*
*  @note
*    - A resource group groups resources (Vulkan descriptor set, Direct3D 12 descriptor table)
*    - A resource group is an instance of a root descriptor table
*    - Descriptor set comes from Vulkan while root descriptor table comes from Direct3D 12; both APIs have a similar but not identical binding model
*    - Overview of the binding models of explicit APIs: "Choosing a binding model" - https://github.com/gpuweb/gpuweb/issues/19
*    - Performance hint: Group resources by binding frequency and set resource groups with a low binding frequency at a low index (e.g. bind a per-pass resource group at index 0)
*    - Compatibility hint: The number of simultaneous bound resource groups is rather low; try to stick to less or equal to four simultaneous bound resource groups, see http://vulkan.gpuinfo.org/listfeatures.php to check out GPU hardware capabilities
*    - Compatibility hint: In Direct3D 12, samplers are not allowed in the same descriptor table as CBV/UAV/SRVs, put them into a sampler resource group
*    - Compatibility hint: In Vulkan, one is usually using a combined image sampler, as a result a sampler resource group doesn't translate to a Vulkan sampler descriptor set.
* Use sampler resource group at the highest binding indices to compensate for this.
*/
class IResourceGroup : public IResource
{
public:
	inline virtual ~IResourceGroup() override
	{
#if SE_STATISTICS
		--GetRender().getStatistics().currentNumberOfResourceGroups;
#endif
	}
protected:
	inline explicit IResourceGroup(IRendererBackend &render SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IResource(ResourceType::RESOURCE_GROUP, render SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		++render.getStatistics().numberOfCreatedResourceGroups;
		++render.getStatistics().currentNumberOfResourceGroups;
#endif
	}

	explicit IResourceGroup(const IResourceGroup&) = delete;
	IResourceGroup& operator=(const IResourceGroup&) = delete;
};

typedef SmartRefCount<IResourceGroup> IResourceGroupPtr;

//[-------------------------------------------------------]
//[ Rhi/Shader/IGraphicsProgram.h                         ]
//[-------------------------------------------------------]
class IGraphicsProgram : public IResource
{
public:
	inline virtual ~IGraphicsProgram() override
	{
#if SE_STATISTICS
		--GetRender().getStatistics().currentNumberOfGraphicsPrograms;
#endif
	}

	[[nodiscard]] inline virtual handle GetUniformHandle([[maybe_unused]] const char* uniformName)
	{
		return SE_NULL_HANDLE;
	}

	inline virtual void SetUniform1i([[maybe_unused]] handle uniformHandle, [[maybe_unused]] int value)
	{
	}

	inline virtual void SetUniform1f([[maybe_unused]] handle uniformHandle, [[maybe_unused]] float value)
	{
	}

	inline virtual void SetUniform2fv([[maybe_unused]] handle uniformHandle, [[maybe_unused]] const float* value)
	{
	}

	inline virtual void SetUniform3fv([[maybe_unused]] handle uniformHandle, [[maybe_unused]] const float* value)
	{
	}

	inline virtual void SetUniform4fv([[maybe_unused]] handle uniformHandle, [[maybe_unused]] const float* value)
	{
	}

	inline virtual void SetUniformMatrix3fv([[maybe_unused]] handle uniformHandle, [[maybe_unused]] const float* value)
	{
	}

	inline virtual void SetUniformMatrix4fv([[maybe_unused]] handle uniformHandle, [[maybe_unused]] const float* value)
	{
	}

protected:
	inline explicit IGraphicsProgram(IRendererBackend &render SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IResource(ResourceType::GRAPHICS_PROGRAM, render SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		++render.getStatistics().numberOfCreatedGraphicsPrograms;
		++render.getStatistics().currentNumberOfGraphicsPrograms;
#endif
	}

	explicit IGraphicsProgram(const IGraphicsProgram&) = delete;
	IGraphicsProgram& operator=(const IGraphicsProgram&) = delete;
};

typedef SmartRefCount<IGraphicsProgram> IGraphicsProgramPtr;

//[-------------------------------------------------------]
//[ Rhi/RenderTarget/IRenderPass.h                        ]
//[-------------------------------------------------------]

class IRenderPass : public IResource
{
public:
	inline virtual ~IRenderPass() override
	{
#if SE_STATISTICS
		--GetRender().getStatistics().currentNumberOfRenderPasses;
#endif
	}

protected:
	inline explicit IRenderPass(IRendererBackend &render SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IResource(ResourceType::RENDER_PASS, render SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		++render.getStatistics().numberOfCreatedRenderPasses;
		++render.getStatistics().currentNumberOfRenderPasses;
#endif
	}

	explicit IRenderPass(const IRenderPass&) = delete;
	IRenderPass& operator=(const IRenderPass&) = delete;
};

typedef SmartRefCount<IRenderPass> IRenderPassPtr;

//[-------------------------------------------------------]
//[ Rhi/RenderTarget/IQueryPool.h                         ]
//[-------------------------------------------------------]
class IQueryPool : public IResource
{
public:
	inline virtual ~IQueryPool() override
	{
#if SE_STATISTICS
		--GetRender().getStatistics().currentNumberOfQueryPools;
#endif
	}

protected:
	inline explicit IQueryPool(IRendererBackend &render SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IResource(ResourceType::QUERY_POOL, render SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		++render.getStatistics().numberOfCreatedQueryPools;
		++render.getStatistics().currentNumberOfQueryPools;
#endif
	}

	explicit IQueryPool(const IQueryPool&) = delete;
	IQueryPool& operator=(const IQueryPool&) = delete;
};

typedef SmartRefCount<IQueryPool> IQueryPoolPtr;

//[-------------------------------------------------------]
//[ Rhi/RenderTarget/IRenderTarget.h                      ]
//[-------------------------------------------------------]
class IRenderTarget : public IResource
{
public:
	inline virtual ~IRenderTarget() override
	{
		m_renderPass.ReleaseReference();
	}

	[[nodiscard]] inline IRenderPass& GetRenderPass() const
	{
		return m_renderPass;
	}

	virtual void GetWidthAndHeight(uint32_t &width, uint32_t &height) const = 0;

protected:
	inline IRenderTarget(ResourceType resourceType, IRenderPass &renderPass SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IResource(resourceType, renderPass.GetRender() SE_DEBUG_PASS_PARAMETER),
		m_renderPass(renderPass)
	{
		m_renderPass.AddReference();
	}

	explicit IRenderTarget(const IRenderTarget&) = delete;
	IRenderTarget& operator=(const IRenderTarget&) = delete;

private:
	IRenderPass &m_renderPass;
};

typedef SmartRefCount<IRenderTarget> IRenderTargetPtr;

//[-------------------------------------------------------]
//[ Rhi/RenderTarget/ISwapChain.h                         ]
//[-------------------------------------------------------]
/**
*  @brief
*    Abstract render window interface which is used to implement some platform specific functionality regarding render window needed by the swap chain implementation
*
*  @remarks
*    This interface can be used to implement the needed platform specific functionality for a platform which isn't known by the RHI implementation e.g. the user uses a windowing library (e.g. SDL2) which abstracts the window handling on different windowing platforms (e.g. Win32 or Linux/Wayland) and the application should run on a windowing platform which isn't supported by the swap chain implementation itself.
*/
class IRenderWindow
{
public:
	inline virtual ~IRenderWindow()
	{
	}

	virtual void GetWidthAndHeight(uint32_t &width, uint32_t &height) const = 0;

	virtual void Present() = 0;

protected:
	inline IRenderWindow()
	{
	}

	explicit IRenderWindow(const IRenderWindow&) = delete;
	IRenderWindow& operator=(const IRenderWindow&) = delete;
};

class ISwapChain : public IRenderTarget
{
public:
	inline virtual ~ISwapChain() override
	{
#if SE_STATISTICS
		--GetRender().getStatistics().currentNumberOfSwapChains;
#endif
	}

	[[nodiscard]] virtual handle GetNativeWindowHandle() const = 0;

	virtual void SetVerticalSynchronizationInterval(uint32_t synchronizationInterval) = 0;

	virtual void Present() = 0;

	virtual void ResizeBuffers() = 0;

	[[nodiscard]] virtual bool GetFullscreenState() const = 0;
	virtual void SetFullscreenState(bool fullscreen) = 0;

	virtual void SetRenderWindow(IRenderWindow *renderWindow) = 0;

protected:
	inline explicit ISwapChain(IRenderPass &renderPass SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) :
		IRenderTarget(ResourceType::SWAP_CHAIN, renderPass SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		++GetRender().getStatistics().numberOfCreatedSwapChains;
		++GetRender().getStatistics().currentNumberOfSwapChains;
#endif
	}

	explicit ISwapChain(const ISwapChain&) = delete;
	ISwapChain& operator=(const ISwapChain&) = delete;
};

typedef SmartRefCount<ISwapChain> ISwapChainPtr;

SE_NAMESPACE_END
//=============================================================================