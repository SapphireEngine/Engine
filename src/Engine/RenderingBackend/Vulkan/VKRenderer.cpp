#include "stdafx.h"
#include "VKRenderer.h"
#include "VKCore.h"
#include "VKRuntimeLinking.h"
#include "Core/Memory/Memory.h"
#include "VKContext.h"
#include "VKSwapChain.h"
#define VKHEADER_IMPL
#include "VKFuncPrototypes.h"
#undef VKHEADER_IMPL

#if SE_VULKAN

#if SE_COMPILER_MSVC
#	pragma comment(lib, "vulkan-1.lib")
#endif

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
VKRenderer::VKRenderer()
	: m_vkAllocationCallbacks{ GetSubsystem<Memory>().DefaultAlloc(), &detail::VKAllocationFunction, &detail::VKReallocationFunction, &detail::VKFreeFunction, nullptr, nullptr }
{
}
//-----------------------------------------------------------------------------
bool VKRenderer::Create(RenderConfig &config, SE_NAMESPACE_WND::Window &window)
{
#if SE_DEBUG
	const bool enableValidation = true;
#else
	const bool enableValidation = false;
#endif

	// Is Vulkan available?
	m_vulkanRuntimeLinking = SE_NEW(VulkanRuntimeLinking)(*this, enableValidation);
	if ( m_vulkanRuntimeLinking->IsVulkanAvaiable() )
	{
		// TODO(co) Add external Vulkan context support
		m_vulkanContext = SE_NEW(VulkanContext)(*this);

		// Is the Vulkan context initialized?
		if ( m_vulkanContext->IsInitialized() )
		{
			// Initialize the capabilities
			initializeCapabilities();
		}
	}

	return true;
}
//-----------------------------------------------------------------------------
void VKRenderer::Destroy()
{
}
//-----------------------------------------------------------------------------
bool VKRenderer::BeginFrame()
{
	return false;
}
//-----------------------------------------------------------------------------
bool VKRenderer::EndFrame()
{
	return false;
}
//-----------------------------------------------------------------------------
void VKRenderer::Resize(unsigned width, unsigned height)
{
}
//-----------------------------------------------------------------------------
void VKRenderer::selfDestruct()
{
	SE_DELETE(VKRenderer, this);
}
//-----------------------------------------------------------------------------
void VKRenderer::initializeCapabilities()
{
	{ // Get device name
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_vulkanContext->GetVkPhysicalDevice(), &vkPhysicalDeviceProperties);
		const size_t numberOfCharacters = Countof(m_capabilities.deviceName) - 1;
		strncpy(m_capabilities.deviceName, vkPhysicalDeviceProperties.deviceName, numberOfCharacters);
		m_capabilities.deviceName[numberOfCharacters] = '\0';
	}

	// Preferred swap chain texture format
	m_capabilities.preferredSwapChainColorTextureFormat = (SwapChain::FindColorVkFormat(m_vulkanRuntimeLinking->GetVkInstance(), *m_vulkanContext) == VK_FORMAT_R8G8B8A8_UNORM) ? TextureFormat::Enum::R8G8B8A8 : TextureFormat::Enum::B8G8R8A8;

	{ // Preferred swap chain depth stencil texture format
		const VkFormat depthVkFormat = SwapChain::FindDepthVkFormat(m_vulkanContext->GetVkPhysicalDevice());
		if ( VK_FORMAT_D32_SFLOAT == depthVkFormat )
		{
			m_capabilities.preferredSwapChainDepthStencilTextureFormat = TextureFormat::Enum::D32_FLOAT;
		}
		else
		{
			// TODO(co) Add support for "VK_FORMAT_D32_SFLOAT_S8_UINT" and "VK_FORMAT_D24_UNORM_S8_UINT"
			m_capabilities.preferredSwapChainDepthStencilTextureFormat = TextureFormat::Enum::D32_FLOAT;
		}
	}

	// TODO(co) Implement me, this in here is just a placeholder implementation

	{ // "D3D_FEATURE_LEVEL_11_0"
		// Maximum number of viewports (always at least 1)
		m_capabilities.maximumNumberOfViewports = 16;

		// Maximum number of simultaneous render targets (if <1 render to texture is not supported)
		m_capabilities.maximumNumberOfSimultaneousRenderTargets = 8;

		// Maximum texture dimension
		m_capabilities.maximumTextureDimension = 16384;

		// Maximum number of 1D texture array slices (usually 512, in case there's no support for 1D texture arrays it's 0)
		m_capabilities.maximumNumberOf1DTextureArraySlices = 512;

		// Maximum number of 2D texture array slices (usually 512, in case there's no support for 2D texture arrays it's 0)
		m_capabilities.maximumNumberOf2DTextureArraySlices = 512;

		// Maximum number of cube texture array slices (usually 512, in case there's no support for cube texture arrays it's 0)
		m_capabilities.maximumNumberOfCubeTextureArraySlices = 512;

		// Maximum texture buffer (TBO) size in texel (>65536, typically much larger than that of one-dimensional texture, in case there's no support for texture buffer it's 0)
		m_capabilities.maximumTextureBufferSize = m_capabilities.maximumStructuredBufferSize = 128 * 1024 * 1024;	// TODO(co) http://msdn.microsoft.com/en-us/library/ff476876%28v=vs.85%29.aspx does not mention the texture buffer? Currently the OpenGL 3 minimum is used: 128 MiB.

		// Maximum indirect buffer size in bytes
		m_capabilities.maximumIndirectBufferSize = 128 * 1024;	// 128 KiB

		// Maximum number of multisamples (always at least 1, usually 8)
		m_capabilities.maximumNumberOfMultisamples = 1;	// TODO(co) Add multisample support, when setting "VkPhysicalDeviceFeatures" -> "sampleRateShading" -> VK_TRUE don't forget to check whether or not the device sup pots the feature

		// Maximum anisotropy (always at least 1, usually 16)
		m_capabilities.maximumAnisotropy = 16;

		// Instanced arrays supported? (shader model 3 feature, vertex array element advancing per-instance instead of per-vertex)
		m_capabilities.instancedArrays = true;

		// Draw instanced supported? (shader model 4 feature, build in shader variable holding the current instance ID)
		m_capabilities.drawInstanced = true;

		// Maximum number of vertices per patch (usually 0 for no tessellation support or 32 which is the maximum number of supported vertices per patch)
		m_capabilities.maximumNumberOfPatchVertices = 32;

		// Maximum number of vertices a geometry shader can emit (usually 0 for no geometry shader support or 1024)
		m_capabilities.maximumNumberOfGsOutputVertices = 1024;	// TODO(co) http://msdn.microsoft.com/en-us/library/ff476876%28v=vs.85%29.aspx does not mention it, so I assume it's 1024
	}

	// The rest is the same for all feature levels

	// Maximum uniform buffer (UBO) size in bytes (usually at least 4096 * 16 bytes, in case there's no support for uniform buffer it's 0)
	// -> See https://msdn.microsoft.com/en-us/library/windows/desktop/ff819065(v=vs.85).aspx - "Resource Limits (Direct3D 11)" - "Number of elements in a constant buffer D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT (4096)"
	// -> One element = float4 = 16 bytes
	m_capabilities.maximumUniformBufferSize = 4096 * 16;

	// Left-handed coordinate system with clip space depth value range 0..1
	m_capabilities.upperLeftOrigin = m_capabilities.zeroToOneClipZ = true;

	// Individual uniforms ("constants" in Direct3D terminology) supported? If not, only uniform buffer objects are supported.
	m_capabilities.individualUniforms = false;

	// Base vertex supported for draw calls?
	m_capabilities.baseVertex = true;

	// Vulkan has native multithreading
	m_capabilities.nativeMultithreading = false;	// TODO(co) Enable native multithreading when done

	// Vulkan has shader bytecode support
	m_capabilities.shaderBytecode = false;	// TODO(co) Vulkan has shader bytecode support, set "m_capabilities.shaderBytecode" to true later on

	// Is there support for vertex shaders (VS)?
	m_capabilities.vertexShader = true;

	// Is there support for fragment shaders (FS)?
	m_capabilities.fragmentShader = true;

	// Is there support for compute shaders (CS)?
	m_capabilities.computeShader = true;
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================

#endif