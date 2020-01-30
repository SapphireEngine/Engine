#pragma once

#include "Core/Template/Ref.h"
#include "RenderingBackend/CoreTypes/CoreTypes.h"
#include "RenderingBackend/CoreSystem/Capabilities.h"
#include "RenderingBackend/CoreSystem/Statistics.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IShaderLanguage;
class IQueryPool;
class ISwapChain;
class IFramebuffer;
class FramebufferAttachment;
class IBufferManager;
class ITextureManager;
class IGraphicsPipelineState;
class IComputePipelineState;
class IComputeShader;
class ISamplerState;
class IResource;
class CommandBuffer;

enum class RenderingBackend
{
	Direct3D11,
	Direct3D12,
	OpenGL,
	Vulkan
};

struct RenderConfig
{
	RenderingBackend render = RenderingBackend::Vulkan;
};

class SE_NO_VTABLE IRendererBackend : public RefCount<IRendererBackend>
{
	friend class IRootSignature;
	friend class IResourceGroup;
	friend class IGraphicsProgram;
	friend class IRenderPass;
	friend class IQueryPool;
	friend class IRenderTarget;
	friend class ISwapChain;
public:
	//[=======================================================================]
	//[ Core                                                                  ]
	//[=======================================================================]

	virtual ~IRendererBackend() = default;

	[[nodiscard]] virtual bool Create(RenderConfig &config) = 0;
	virtual void Destroy() = 0;

	[[nodiscard]] RenderingBackend GetRenderingBackendType() const {return m_config.render; }

	[[nodiscard]] inline const Capabilities& GetCapabilities() const
	{
		return m_capabilities;
	}

	[[nodiscard]] inline IShaderLanguage& GetDefaultShaderLanguage()
	{
		IShaderLanguage *shaderLanguage = GetShaderLanguage();
		SE_ASSERT(nullptr != shaderLanguage, "There's no default shader language");
		return *shaderLanguage;
	}

#if SE_STATISTICS
	[[nodiscard]] inline const Statistics& GetStatistics()
	{
		return m_statistics;
	}
#endif

	[[nodiscard]] virtual const char* GetName() const = 0;

	[[nodiscard]] virtual bool IsInitialized() const = 0;

	/**
	*    it is possible to check whether or not your application is currently running
	*    within a known debug/profile tool like e.g. Direct3D PIX (also works directly within VisualStudio
	*    2017 out-of-the-box). In case you want at least try to protect your asset, you might want to stop
	*    the execution of your application when a debug/profile tool is used which can e.g. record your data.
	*    Please be aware that this will only make it a little bit harder to debug and e.g. while doing so
	*    reading out your asset data. Public articles like
	*    "PIX: How to circumvent D3DPERF_SetOptions" at
	*      http://www.gamedev.net/blog/1323/entry-2250952-pix-how-to-circumvent-d3dperf-setoptions/
	*    describe how to "hack around" this security measurement, so, don't rely on it. Those debug
	*    methods work fine when using a Direct3D RHI implementation. OpenGL on the other hand
	*    has no Direct3D PIX like functions or extensions, use for instance "gDEBugger" (http://www.gremedy.com/)
	*    instead.
	*    -> When using Direct3D <11.1, those methods map to the Direct3D 9 PIX functions (D3DPERF_* functions)
	*    -> The Direct3D 9 PIX functions are also used for Direct3D 10 and Direct3D 11. Lookout! As soon as using
	*       the debug methods within this interface, the Direct3D 9 dll will be loaded.
	*    -> Starting with Direct3D 11.1, the Direct3D 9 PIX functions no longer work. Instead, the new
	*       "D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY"-flag (does not work with <Direct3D 11.1)
	*       is used when creating the device instance, then the "ID3DUserDefinedAnnotation"-API is used.
	*    -> Optimization: You might want to use those methods only via macros to make it easier to avoid using them within e.g. a final release build
	*/
	[[nodiscard]] virtual bool IsDebugEnabled() = 0;

	//[=======================================================================]
	//[ Shader language                                                       ]
	//[=======================================================================]

	// Return the number of supported shader languages
	[[nodiscard]] virtual uint32_t GetNumberOfShaderLanguages() const = 0;

	// Return the name of a supported shader language at the provided index
	// index - Index of the supported shader language to return the name from ([0, getNumberOfShaderLanguages()-1])
	// return - The ASCII name (for example "GLSL" or "HLSL") of the supported shader language at the provided index, can be a null pointer
	[[nodiscard]] virtual const char* GetShaderLanguageName(uint32_t index) const = 0;

	/**
	*  @brief
	*    Return a shader language instance
	*
	*  @param[in] shaderLanguageName
	*    The ASCII name of the shader language (for example "GLSL" or "HLSL"), if null pointer or empty string,
	*    the default RHI shader language is used (see "GetShaderLanguageName(0)")
	*
	*  @return
	*    The shader language instance, a null pointer on error, do not release the returned instance unless you added an own reference to it
	*/
	[[nodiscard]] virtual IShaderLanguage* GetShaderLanguage(const char *shaderLanguageName = nullptr) = 0;

	//[=======================================================================]
	//[ Resource creation                                                     ]
	//[=======================================================================]

	/**
	*  @brief
	*    Create a render pass instance
	*
	*  @param[in] numberOfColorAttachments
	*    Number of color render target textures, must be <="Capabilities::maximumNumberOfSimultaneousRenderTargets"
	*  @param[in] colorAttachmentTextureFormats
	*    The color render target texture formats, can be a null pointer or can contain null pointers, if not a null pointer there must be at
	*    least "numberOfColorAttachments" textures in the provided C-array of pointers
	*  @param[in] depthStencilAttachmentTextureFormat
	*    The optional depth stencil render target texture format, can be a "TextureFormat::UNKNOWN" if there should be no depth buffer
	*  @param[in] numberOfMultisamples
	*    The number of multisamples per pixel (valid values: 1, 2, 4, 8)
	*
	*  @return
	*    The created render pass instance, null pointer on error. Release the returned instance if you no longer need it.
	*/
	[[nodiscard]] virtual IRenderPass* CreateRenderPass(uint32_t numberOfColorAttachments, const TextureFormat::Enum* colorAttachmentTextureFormats, TextureFormat::Enum depthStencilAttachmentTextureFormat = TextureFormat::UNKNOWN, uint8_t numberOfMultisamples = 1 SE_DEBUG_NAME_PARAMETER) = 0;

	/**
	*  @brief
	*    Create a asynchronous query pool instance
	*
	*  @param[in] queryType
	*    Query type (e.g. "QueryType::OCCLUSION")
	*  @param[in] numberOfQueries
	*    The number of queries the query pool contains (e.g. 1)
	*
	*  @return
	*    The created query pool instance, null pointer on error. Release the returned instance if you no longer need it.
	*/
	[[nodiscard]] virtual IQueryPool* CreateQueryPool(QueryType queryType, uint32_t numberOfQueries = 1 SE_DEBUG_NAME_PARAMETER) = 0;

	/**
	*  @brief
	*    Create a swap chain instance
	*
	*  @param[in] renderPass
	*    Render pass to use, the swap chain keeps a reference to the render pass
	*  @param[in] windowHandle
	*    Information about the window to render into
	*  @param[in] useExternalContext
	*    Indicates if an external RHI context is used; in this case the RHI itself has nothing to do with the creation/managing of an RHI context
	*
	*  @return
	*    The created swap chain instance, null pointer on error. Release the returned instance if you no longer need it.
	*/
	[[nodiscard]] virtual ISwapChain* CreateSwapChain(IRenderPass &renderPass, bool useExternalContext = false SE_DEBUG_NAME_PARAMETER) = 0;

	/**
	*  @brief
	*    Create a framebuffer object (FBO) instance
	*
	*  @param[in] renderPass
	*    Render pass to use, the framebuffer keeps a reference to the render pass
	*  @param[in] colorFramebufferAttachments
	*    The color render target textures, can be a null pointer or can contain null pointers, if not a null pointer there must be at
	*    least "IRenderPass::getNumberOfColorAttachments()" textures in the provided C-array of pointers
	*  @param[in] depthStencilFramebufferAttachment
	*    The optional depth stencil render target texture, can be a null pointer
	*
	*  @return
	*    The created FBO instance, null pointer on error. Release the returned instance if you no longer need it.
	*
	*  @note
	*    - Only supported if "Capabilities::maximumNumberOfSimultaneousRenderTargets" is not 0
	*    - The framebuffer keeps a reference to the provided texture instances
	*    - It's invalid to set the same color texture to multiple render targets at one and the same time
	*    - Depending on the used RHI implementation and feature set, there might be the requirement that all provided textures have the same size
	*      (in order to be on the save side, ensure that all provided textures have the same size and same MSAA sample count)
	*/
	[[nodiscard]] virtual IFramebuffer* CreateFramebuffer(IRenderPass &renderPass, const FramebufferAttachment *colorFramebufferAttachments, const FramebufferAttachment *depthStencilFramebufferAttachment = nullptr SE_DEBUG_NAME_PARAMETER) = 0;

	[[nodiscard]] virtual IBufferManager* CreateBufferManager() = 0;

	[[nodiscard]] virtual ITextureManager* CreateTextureManager() = 0;

	[[nodiscard]] virtual IRootSignature* CreateRootSignature(const RootSignature &rootSignature SE_DEBUG_NAME_PARAMETER) = 0;

	[[nodiscard]] virtual IGraphicsPipelineState* CreateGraphicsPipelineState(const GraphicsPipelineState &graphicsPipelineState SE_DEBUG_NAME_PARAMETER) = 0;

	[[nodiscard]] virtual IComputePipelineState* CreateComputePipelineState(IRootSignature &rootSignature, IComputeShader& computeShader SE_DEBUG_NAME_PARAMETER) = 0;

	[[nodiscard]] virtual ISamplerState* CreateSamplerState(const SamplerState &samplerState SE_DEBUG_NAME_PARAMETER) = 0;

	//[=======================================================================]
	//[ Resource handling                                                     ]
	//[=======================================================================]

	[[nodiscard]] virtual bool Map(IResource &resource, uint32_t subresource, MapType mapType, uint32_t mapFlags, MappedSubresource &mappedSubresource) = 0;

	virtual void Unmap(IResource &resource, uint32_t subresource) = 0;

	// Get asynchronous query pool results
	[[nodiscard]] virtual bool GetQueryPoolResults(IQueryPool &queryPool, uint32_t numberOfDataBytes, uint8_t *data, uint32_t firstQueryIndex = 0, uint32_t numberOfQueries = 1, uint32_t strideInBytes = 0, uint32_t queryResultFlags = QueryResultFlags::WAIT) = 0;

	//[=======================================================================]
	//[ Operations                                                            ]
	//[=======================================================================]
	
	[[nodiscard]] virtual bool BeginScene() = 0;

	virtual void SubmitCommandBuffer(const CommandBuffer &commandBuffer) = 0;

	virtual void EndScene() = 0;

	//[=======================================================================]
	//[ Synchronization                                                       ]
	//[=======================================================================]

	// Force the execution of render commands in finite time (synchronization)
	virtual void Flush() = 0;

	// Force the execution of render commands in finite time and wait until it's done (synchronization)
	virtual void Finish() = 0;

	//[=======================================================================]
	//[ Render implementation specific                                        ]
	//[=======================================================================]
	[[nodiscard]] virtual void* GetD3D11DevicePointer() const
	{
		return nullptr;
	}

	[[nodiscard]] virtual void* GetD3D11ImmediateContextPointer() const
	{
		return nullptr;
	}

protected:
	explicit IRendererBackend(const IRendererBackend&) = delete;
	IRendererBackend& operator=(const IRendererBackend&) = delete;

#if SE_STATISTICS
	[[nodiscard]] inline Statistics& getStatistics()
	{
		return m_statistics;
	}
#endif
	RenderConfig m_config;
	Capabilities m_capabilities;
#if SE_STATISTICS
	Statistics m_statistics;
#endif
};

SE_NAMESPACE_END
//=============================================================================