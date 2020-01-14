#pragma once

#include "RenderingBackend/Direct3D11.h"
#include "RenderingBackend/Direct3D12.h"
#include "RenderingBackend/OpenGL.h"
#include "RenderingBackend/Vulkan.h"

//=============================================================================
SE_NAMESPACE_BEGIN

// Common Utils

inline std::vector<char> readFile(const std::string& filename)
{
	std::string path = filename;
	char pBuf[1024];
#if SE_PLATFORM_WINDOWS

	_getcwd(pBuf, 1024);
	path = pBuf;
	path += "\\";
#else
	getcwd(pBuf, 1024);
	path = pBuf;
	path += "/";
#endif
	path += filename;
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	bool exists = (bool)file;

	if ( !exists || !file.is_open() )
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
};

template <typename T>
inline T clamp(const T& value, const T& low, const T& high)
{
	return value < low ? low : (value > high ? high : value);
}

class ORenderer
{
public:
	ORenderer(SE_NAMESPACE_WND::Window& window);
	~ORenderer();

	void render();

	void resize(unsigned width, unsigned height);

protected:

	// Initialize your Graphics API
	void initializeAPI(SE_NAMESPACE_WND::Window& window);

	// Destroy any Graphics API data structures used in this example
	void destroyAPI();

	// Initialize any resources such as VBOs, IBOs, used in this example
	void initializeResources();

	// Destroy any resources used in this example
	void destroyResources();

	// Create graphics API specific data structures to send commands to the GPU
	void createCommands();

	// Set up commands used when rendering frame by this app
	void setupCommands();

	// Destroy all commands
	void destroyCommands();

	// Set up the FrameBuffer
	void initFrameBuffer();

	void destroyFrameBuffer();

	// Set up the RenderPass
	void createRenderPass();

	void createSynchronization();

	// Set up the swapchain
	void setupSwapchain(unsigned width, unsigned height);

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	Vertex mVertexBufferData[3] =
	{
	  { { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
	  { { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
	  { { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};

	uint32_t mIndexBufferData[3] = { 0, 1, 2 };

	std::chrono::time_point<std::chrono::steady_clock> tStart, tEnd;
	float mElapsedTime = 0.0f;

	// Uniform data
	struct
	{
		Matrix4 projectionMatrix;
		Matrix4 modelMatrix;
		Matrix4 viewMatrix;
	} uboVS;

#if SE_VULKAN && 1
	// Initialization
	vk::Instance mInstance;
	vk::PhysicalDevice mPhysicalDevice;
	vk::Device mDevice;

	vk::SwapchainKHR mSwapchain;
	vk::SurfaceKHR mSurface;

	float mQueuePriority;
	vk::Queue mQueue;
	uint32_t mQueueFamilyIndex;

	vk::CommandPool mCommandPool;
	std::vector<vk::CommandBuffer> mCommandBuffers;
	uint32_t mCurrentBuffer;

	vk::Extent2D mSurfaceSize;
	vk::Rect2D mRenderArea;
	vk::Viewport mViewport;

	// Resources
	vk::Format mSurfaceColorFormat;
	vk::ColorSpaceKHR mSurfaceColorSpace;
	vk::Format mSurfaceDepthFormat;
	vk::Image mDepthImage;
	vk::DeviceMemory mDepthImageMemory;

	vk::DescriptorPool mDescriptorPool;
	std::vector<vk::DescriptorSetLayout> mDescriptorSetLayouts;
	std::vector<vk::DescriptorSet> mDescriptorSets;

	vk::ShaderModule mVertModule;
	vk::ShaderModule mFragModule;

	vk::RenderPass mRenderPass;

	vk::Buffer mVertexBuffer;
	vk::Buffer mIndexBuffer;

	vk::PipelineCache mPipelineCache;
	vk::Pipeline mPipeline;
	vk::PipelineLayout mPipelineLayout;

	// Sync
	vk::Semaphore mPresentCompleteSemaphore;
	vk::Semaphore mRenderCompleteSemaphore;
	std::vector<vk::Fence> mWaitFences;

	// Swpachain
	struct SwapChainBuffer
	{
		vk::Image image;
		std::array<vk::ImageView, 2> views;
		vk::Framebuffer frameBuffer;
	};

	std::vector<SwapChainBuffer> mSwapchainBuffers;

	// Vertex buffer and attributes
	struct
	{
		vk::DeviceMemory memory;															// Handle to the device memory for this buffer
		vk::Buffer buffer;																// Handle to the Vulkan buffer object that the memory is bound to
		vk::PipelineVertexInputStateCreateInfo inputState;
		vk::VertexInputBindingDescription inputBinding;
		std::vector<vk::VertexInputAttributeDescription> inputAttributes;
	} mVertices;

	// Index buffer
	struct
	{
		vk::DeviceMemory memory;
		vk::Buffer buffer;
		uint32_t count;
	} mIndices;

	// Uniform block object
	struct
	{
		vk::DeviceMemory memory;
		vk::Buffer buffer;
		vk::DescriptorBufferInfo descriptor;
	}  mUniformDataVS;

#elif SE_DIRECT3D12 && 0

	static const UINT backbufferCount = 2;

	SE_NAMESPACE_WND::Window* mWindow;
	unsigned mWidth, mHeight;

	// Initialization
	IDXGIFactory4* mFactory;
	IDXGIAdapter1* mAdapter;
#if SE_DEBUG
	ID3D12Debug1* mDebugController;
	ID3D12DebugDevice* mDebugDevice;
#endif
	ID3D12Device* mDevice;
	ID3D12CommandQueue* mCommandQueue;
	ID3D12CommandAllocator* mCommandAllocator;
	ID3D12GraphicsCommandList* mCommandList;

	// Current Frame
	UINT mCurrentBuffer;
	ID3D12DescriptorHeap* mRtvHeap;
	ID3D12Resource* mRenderTargets[backbufferCount];
	IDXGISwapChain3* mSwapchain;

	// Resources
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mSurfaceSize;

	ID3D12Resource* mVertexBuffer;
	ID3D12Resource* mIndexBuffer;

	ID3D12Resource* mUniformBuffer;
	ID3D12DescriptorHeap* mUniformBufferHeap;
	UINT8* mMappedUniformBuffer;

	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	UINT mRtvDescriptorSize;
	ID3D12RootSignature* mRootSignature;
	ID3D12PipelineState* mPipelineState;

	// Sync
	UINT mFrameIndex;
	HANDLE mFenceEvent;
	ID3D12Fence* mFence;
	UINT64 mFenceValue;

#elif SE_DIRECT3D11 && 0

	bool mVsync;
	SE_NAMESPACE_WND::Window* mWindow;
	unsigned mWidth, mHeight;
	D3D11_VIEWPORT mViewport;

	IDXGIFactory* mFactory;
	IDXGIAdapter* mAdapter;
#if SE_DEBUG
	ID3D11Debug* mDebugController;
#endif
	IDXGIOutput* mAdapterOutput;
	unsigned mNumerator, mDenominator;
	ID3D11Device* mDevice;
	ID3D11DeviceContext* mDeviceContext;

	IDXGISwapChain* mSwapchain;
	ID3D11Texture2D* mBackbufferTex;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11DepthStencilView* mDepthStencilView;
	ID3D11RenderTargetView* mRenderTargetView;

	ID3D11DepthStencilState* mDepthStencilState;
	ID3D11RasterizerState* mRasterState;

	ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
	ID3D11InputLayout* mLayout;
	ID3D11Buffer* mUniformBuffer;
	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;

#elif SE_OPENGL && 0
	//Initialization
	OpenGLState mOGLState;

	unsigned mWidth, mHeight;

	GLuint mFrameBuffer;
	GLuint mFrameBufferTex;
	GLuint mRenderBufferDepth;

	// Resources
	GLuint mVertexShader;
	GLuint mFragmentShader;
	GLuint mProgram;
	GLuint mVertexArray;
	GLuint mVertexBuffer;
	GLuint mIndexBuffer;

	GLuint mUniformUBO;

	GLint mPositionAttrib;
	GLint mColorAttrib;
#endif
};

SE_NAMESPACE_END
//=============================================================================