#pragma once

#include "Platform/WindowsCore.h"

//
// default capability levels of the renderer
//
#if !defined(RENDERER_CUSTOM_MAX)
enum
{
	MAX_INSTANCE_EXTENSIONS = 64,
	MAX_DEVICE_EXTENSIONS = 64,
	MAX_GPUS = 10,
	MAX_RENDER_TARGET_ATTACHMENTS = 8,
	MAX_SUBMIT_CMDS = 20,    // max number of command lists / command buffers
	MAX_SUBMIT_WAIT_SEMAPHORES = 8,
	MAX_SUBMIT_SIGNAL_SEMAPHORES = 8,
	MAX_PRESENT_WAIT_SEMAPHORES = 8,
	MAX_VERTEX_BINDINGS = 15,
	MAX_VERTEX_ATTRIBS = 15,
	MAX_SEMANTIC_NAME_LENGTH = 128,
	MAX_DEBUG_NAME_LENGTH = 128,
	MAX_MIP_LEVELS = 0xFFFFFFFF,
	MAX_SWAPCHAIN_IMAGES = 3,
	MAX_ROOT_CONSTANTS_PER_ROOTSIGNATURE = 4,
	MAX_GPU_VENDOR_STRING_LENGTH = 64    //max size for GPUVendorPreset strings
};
#endif

#if defined(DIRECT3D11)
#	include <d3d11_1.h>
#	include <dxgi1_2.h>
#endif
#if defined(DIRECT3D12)
#	include <d3d12.h>
#	include <dxgi1_6.h>
#	include <dxgidebug.h>
#endif
#if defined(VULKAN)
#	if defined(_WIN32)
#		define VK_USE_PLATFORM_WIN32_KHR
#	elif defined(__ANDROID__)
#		ifndef VK_USE_PLATFORM_ANDROID_KHR
#			define VK_USE_PLATFORM_ANDROID_KHR
#		endif
#	elif defined(__linux__) && !defined(VK_USE_PLATFORM_GGP)
#		define VK_USE_PLATFORM_XLIB_KHR    //Use Xlib or Xcb as display server, defaults to Xlib
#	endif
#	include <volk/volk.h>
#endif

#if defined(VULKAN)
// Set this define to enable renderdoc layer
// NOTE: Setting this define will disable use of the khr dedicated allocation extension since it conflicts with the renderdoc capture layer
//#define USE_RENDER_DOC

// Raytracing
#ifdef VK_NV_RAY_TRACING_SPEC_VERSION
#define ENABLE_RAYTRACING
#endif
#elif defined(DIRECT3D12)
//#define USE_PIX

// Raytracing
#ifdef D3D12_RAYTRACING_AABB_BYTE_ALIGNMENT
#define ENABLE_RAYTRACING
#endif

// Forward declare memory allocator classes
namespace D3D12MA
{
	class Allocator;
	class Allocation;
};
#endif

#include "Platform/Thread.h"
#include <tinyimageformat/tinyimageformat_base.h>

#ifdef __cplusplus
#ifndef MAKE_ENUM_FLAG
#define MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)                                                                        \
	static inline ENUM_TYPE operator|(ENUM_TYPE a, ENUM_TYPE b) { return (ENUM_TYPE)((TYPE)(a) | (TYPE)(b)); } \
	static inline ENUM_TYPE operator&(ENUM_TYPE a, ENUM_TYPE b) { return (ENUM_TYPE)((TYPE)(a) & (TYPE)(b)); } \
	static inline ENUM_TYPE operator|=(ENUM_TYPE& a, ENUM_TYPE b) { return a = (a | b); }                      \
	static inline ENUM_TYPE operator&=(ENUM_TYPE& a, ENUM_TYPE b) { return a = (a & b); }

#endif
#else
#define MAKE_ENUM_FLAG(TYPE, ENUM_TYPE)
#endif

struct DescriptorPool;
struct VmaAllocator_T;
struct VmaAllocation_T;


//=============================================================================
SE_NAMESPACE_BEGIN

typedef enum RendererApi
{
	RENDERER_API_D3D12 = 0,
	RENDERER_API_VULKAN,
	RENDERER_API_METAL,
	RENDERER_API_XBOX_D3D12,
	RENDERER_API_D3D11,
	RENDERER_API_ORBIS
} RendererApi;

typedef enum LogType
{
	LOG_TYPE_INFO = 0,
	LOG_TYPE_WARN,
	LOG_TYPE_DEBUG,
	LOG_TYPE_ERROR
} LogType;

typedef enum QueueType
{
	QUEUE_TYPE_GRAPHICS = 0,
	QUEUE_TYPE_TRANSFER,
	QUEUE_TYPE_COMPUTE,
	MAX_QUEUE_TYPE
} QueueType;

typedef enum QueueFlag
{
	QUEUE_FLAG_NONE = 0x0,
	QUEUE_FLAG_DISABLE_GPU_TIMEOUT = 0x1,
	QUEUE_FLAG_INIT_MICROPROFILE = 0x2,
	MAX_QUEUE_FLAG = 0xFFFFFFFF
} QueueFlag;
MAKE_ENUM_FLAG(uint32_t, QueueFlag)

typedef enum QueuePriority
{
	QUEUE_PRIORITY_NORMAL,
	QUEUE_PRIORITY_HIGH,
	QUEUE_PRIORITY_GLOBAL_REALTIME,
	MAX_QUEUE_PRIORITY
} QueuePriority;

typedef enum LoadActionType
{
	LOAD_ACTION_DONTCARE,
	LOAD_ACTION_LOAD,
	LOAD_ACTION_CLEAR,
	MAX_LOAD_ACTION
} LoadActionType;

typedef void(*LogFn)(LogType, const char*, const char*);

typedef enum ResourceState
{
	RESOURCE_STATE_UNDEFINED = 0,
	RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
	RESOURCE_STATE_INDEX_BUFFER = 0x2,
	RESOURCE_STATE_RENDER_TARGET = 0x4,
	RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
	RESOURCE_STATE_DEPTH_WRITE = 0x10,
	RESOURCE_STATE_DEPTH_READ = 0x20,
	RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
	RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
	RESOURCE_STATE_STREAM_OUT = 0x100,
	RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
	RESOURCE_STATE_COPY_DEST = 0x400,
	RESOURCE_STATE_COPY_SOURCE = 0x800,
	RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
	RESOURCE_STATE_PRESENT = 0x1000,
	RESOURCE_STATE_COMMON = 0x2000,
} ResourceState;
MAKE_ENUM_FLAG(uint32_t, ResourceState)

/// Choosing Memory Type
typedef enum ResourceMemoryUsage
{
	/// No intended memory usage specified.
	RESOURCE_MEMORY_USAGE_UNKNOWN = 0,
	/// Memory will be used on device only, no need to be mapped on host.
	RESOURCE_MEMORY_USAGE_GPU_ONLY = 1,
	/// Memory will be mapped on host. Could be used for transfer to device.
	RESOURCE_MEMORY_USAGE_CPU_ONLY = 2,
	/// Memory will be used for frequent (dynamic) updates from host and reads on device.
	RESOURCE_MEMORY_USAGE_CPU_TO_GPU = 3,
	/// Memory will be used for writing on device and readback on host.
	RESOURCE_MEMORY_USAGE_GPU_TO_CPU = 4,
	RESOURCE_MEMORY_USAGE_COUNT,
	RESOURCE_MEMORY_USAGE_MAX_ENUM = 0x7FFFFFFF
} ResourceMemoryUsage;

// Forward declarations
typedef struct Renderer              Renderer;
typedef struct Queue                 Queue;
typedef struct Pipeline              Pipeline;
typedef struct Buffer                Buffer;
typedef struct Texture               Texture;
typedef struct RenderTarget          RenderTarget;
typedef struct ShaderReflectionInfo  ShaderReflectionInfo;
typedef struct Shader                Shader;
typedef struct DescriptorSet         DescriptorSet;
typedef struct DescriptorIndexMap    DescriptorIndexMap;
typedef struct ShaderDescriptors     ShaderDescriptors;

// Raytracing
typedef struct Raytracing            Raytracing;
typedef struct RaytracingHitGroup    RaytracingHitGroup;
typedef struct AccelerationStructure AccelerationStructure;

typedef struct EsramManager          EsramManager;

typedef struct IndirectDrawArguments
{
	uint32_t mVertexCount;
	uint32_t mInstanceCount;
	uint32_t mStartVertex;
	uint32_t mStartInstance;
} IndirectDrawArguments;

typedef struct IndirectDrawIndexArguments
{
	uint32_t mIndexCount;
	uint32_t mInstanceCount;
	uint32_t mStartIndex;
	uint32_t mVertexOffset;
	uint32_t mStartInstance;
} IndirectDrawIndexArguments;

typedef struct IndirectDispatchArguments
{
	uint32_t mGroupCountX;
	uint32_t mGroupCountY;
	uint32_t mGroupCountZ;
} IndirectDispatchArguments;

typedef enum IndirectArgumentType
{
	INDIRECT_DRAW,
	INDIRECT_DRAW_INDEX,
	INDIRECT_DISPATCH,
	INDIRECT_VERTEX_BUFFER,
	INDIRECT_INDEX_BUFFER,
	INDIRECT_CONSTANT,
	INDIRECT_DESCRIPTOR_TABLE,        // only for vulkan
	INDIRECT_PIPELINE,                // only for vulkan now, probally will add to dx when it comes to xbox
	INDIRECT_CONSTANT_BUFFER_VIEW,    // only for dx
	INDIRECT_SHADER_RESOURCE_VIEW,    // only for dx
	INDIRECT_UNORDERED_ACCESS_VIEW,   // only for dx
	INDIRECT_COMMAND_BUFFER,          // metal ICB
	INDIRECT_COMMAND_BUFFER_OPTIMIZE  // metal indirect buffer optimization
} IndirectArgumentType;
/************************************************/

typedef enum DescriptorType
{
	DESCRIPTOR_TYPE_UNDEFINED = 0,
	DESCRIPTOR_TYPE_SAMPLER = 0x01,
	// SRV Read only texture
	DESCRIPTOR_TYPE_TEXTURE = (DESCRIPTOR_TYPE_SAMPLER << 1),
	/// UAV Texture
	DESCRIPTOR_TYPE_RW_TEXTURE = (DESCRIPTOR_TYPE_TEXTURE << 1),
	// SRV Read only buffer
	DESCRIPTOR_TYPE_BUFFER = (DESCRIPTOR_TYPE_RW_TEXTURE << 1),
	DESCRIPTOR_TYPE_BUFFER_RAW = (DESCRIPTOR_TYPE_BUFFER | (DESCRIPTOR_TYPE_BUFFER << 1)),
	/// UAV Buffer
	DESCRIPTOR_TYPE_RW_BUFFER = (DESCRIPTOR_TYPE_BUFFER << 2),
	DESCRIPTOR_TYPE_RW_BUFFER_RAW = (DESCRIPTOR_TYPE_RW_BUFFER | (DESCRIPTOR_TYPE_RW_BUFFER << 1)),
	/// Uniform buffer
	DESCRIPTOR_TYPE_UNIFORM_BUFFER = (DESCRIPTOR_TYPE_RW_BUFFER << 2),
	/// Push constant / Root constant
	DESCRIPTOR_TYPE_ROOT_CONSTANT = (DESCRIPTOR_TYPE_UNIFORM_BUFFER << 1),
	/// IA
	DESCRIPTOR_TYPE_VERTEX_BUFFER = (DESCRIPTOR_TYPE_ROOT_CONSTANT << 1),
	DESCRIPTOR_TYPE_INDEX_BUFFER = (DESCRIPTOR_TYPE_VERTEX_BUFFER << 1),
	DESCRIPTOR_TYPE_INDIRECT_BUFFER = (DESCRIPTOR_TYPE_INDEX_BUFFER << 1),
	/// Cubemap SRV
	DESCRIPTOR_TYPE_TEXTURE_CUBE = (DESCRIPTOR_TYPE_TEXTURE | (DESCRIPTOR_TYPE_INDIRECT_BUFFER << 1)),
	/// RTV / DSV per mip slice
	DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES = (DESCRIPTOR_TYPE_INDIRECT_BUFFER << 2),
	/// RTV / DSV per array slice
	DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES = (DESCRIPTOR_TYPE_RENDER_TARGET_MIP_SLICES << 1),
	/// RTV / DSV per depth slice
	DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES = (DESCRIPTOR_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
	DESCRIPTOR_TYPE_RAY_TRACING = (DESCRIPTOR_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
#if defined(VULKAN)
	/// Subpass input (descriptor type only available in Vulkan)
	DESCRIPTOR_TYPE_INPUT_ATTACHMENT = (DESCRIPTOR_TYPE_RAY_TRACING << 1),
	DESCRIPTOR_TYPE_TEXEL_BUFFER = (DESCRIPTOR_TYPE_INPUT_ATTACHMENT << 1),
	DESCRIPTOR_TYPE_RW_TEXEL_BUFFER = (DESCRIPTOR_TYPE_TEXEL_BUFFER << 1),
#endif
#if defined(METAL)
	DESCRIPTOR_TYPE_ARGUMENT_BUFFER = (DESCRIPTOR_TYPE_RAY_TRACING << 1),
	DESCRIPTOR_TYPE_INDIRECT_COMMAND_BUFFER = (DESCRIPTOR_TYPE_ARGUMENT_BUFFER << 1),
	DESCRIPTOR_TYPE_RENDER_PIPELINE_STATE = (DESCRIPTOR_TYPE_INDIRECT_COMMAND_BUFFER << 1),
#endif
} DescriptorType;
MAKE_ENUM_FLAG(uint32_t, DescriptorType)

typedef enum SampleCount
{
	SAMPLE_COUNT_1 = 1,
	SAMPLE_COUNT_2 = 2,
	SAMPLE_COUNT_4 = 4,
	SAMPLE_COUNT_8 = 8,
	SAMPLE_COUNT_16 = 16,
} SampleCount;

#ifdef METAL
typedef enum ShaderStage
{
	SHADER_STAGE_NONE = 0,
	SHADER_STAGE_VERT = 0X00000001,
	SHADER_STAGE_FRAG = 0X00000002,
	SHADER_STAGE_COMP = 0X00000004,
	SHADER_STAGE_ALL_GRAPHICS = ((uint32_t)SHADER_STAGE_VERT | (uint32_t)SHADER_STAGE_FRAG),
	SHADER_STAGE_COUNT = 3,
} ShaderStage;
#else
typedef enum ShaderStage
{
	SHADER_STAGE_NONE = 0,
	SHADER_STAGE_VERT = 0X00000001,
	SHADER_STAGE_TESC = 0X00000002,
	SHADER_STAGE_TESE = 0X00000004,
	SHADER_STAGE_GEOM = 0X00000008,
	SHADER_STAGE_FRAG = 0X00000010,
	SHADER_STAGE_COMP = 0X00000020,
	SHADER_STAGE_RAYTRACING = 0X00000040,
	SHADER_STAGE_ALL_GRAPHICS = ((uint32_t)SHADER_STAGE_VERT | (uint32_t)SHADER_STAGE_TESC | (uint32_t)SHADER_STAGE_TESE | (uint32_t)SHADER_STAGE_GEOM | (uint32_t)SHADER_STAGE_FRAG),
	SHADER_STAGE_HULL = SHADER_STAGE_TESC,
	SHADER_STAGE_DOMN = SHADER_STAGE_TESE,
	SHADER_STAGE_COUNT = 7,
} ShaderStage;
#endif
MAKE_ENUM_FLAG(uint32_t, ShaderStage)

//////////////////////////
// BEGIN IShaderReflection
static const uint32_t MAX_SHADER_STAGE_COUNT = 5;

typedef enum TextureDimension
{
	TEXTURE_DIM_1D,
	TEXTURE_DIM_2D,
	TEXTURE_DIM_2DMS,
	TEXTURE_DIM_3D,
	TEXTURE_DIM_CUBE,
	TEXTURE_DIM_1D_ARRAY,
	TEXTURE_DIM_2D_ARRAY,
	TEXTURE_DIM_2DMS_ARRAY,
	TEXTURE_DIM_CUBE_ARRAY,
	TEXTURE_DIM_COUNT,
	TEXTURE_DIM_UNDEFINED,
} TextureDimension;

struct VertexInput
{
	// The size of the attribute
	uint32_t size;

	// resource name
	const char* name;

	// name size
	uint32_t name_size;
};

#if defined(METAL)
struct ArgumentDescriptor
{
	MTLDataType         mDataType;
	uint32_t            mBufferIndex;
	uint32_t            mArgumentIndex;
	uint32_t            mArrayLength;
	MTLArgumentAccess   mAccessType;
	MTLTextureType      mTextureType;
	size_t              mAlignment;
};
#endif

struct ShaderResource
{
	// resource Type
	DescriptorType type;

	// The resource set for binding frequency
	uint32_t set;

	// The resource binding location
	uint32_t reg;

	// The size of the resource. This will be the DescriptorInfo array size for textures
	uint32_t size;

	// what stages use this resource
	ShaderStage used_stages;

	// resource name
	const char* name;

	// name size
	uint32_t name_size;

	// 1D / 2D / Array / MSAA / ...
	TextureDimension dim;

#if defined(METAL)
	uint32_t            alignment;

	uint32_t            mtlTextureType;           // Needed to bind different types of textures as default resources on Metal.
//    uint32_t            mtlArgumentBufferType;    // Needed to bind multiple resources under a same descriptor on Metal.
	bool                mIsArgumentBufferField;
	ArgumentDescriptor  mtlArgumentDescriptors;
#endif
};

struct ShaderVariable
{
	// parents resource index
	uint32_t parent_index;

	// The offset of the Variable.
	uint32_t offset;

	// The size of the Variable.
	uint32_t size;

	// Variable name
	const char* name;

	// name size
	uint32_t name_size;
};

struct ShaderReflection
{
	ShaderStage mShaderStage;

	// single large allocation for names to reduce number of allocations
	char*    pNamePool;
	uint32_t mNamePoolSize;

	VertexInput* pVertexInputs;
	uint32_t     mVertexInputsCount;

	ShaderResource* pShaderResources;
	uint32_t        mShaderResourceCount;

	ShaderVariable* pVariables;
	uint32_t        mVariableCount;

	// Thread group size for compute shader
	uint32_t mNumThreadsPerGroup[3];

	//number of tessellation control point
	uint32_t mNumControlPoint;

#if defined(VULKAN)
	char* pEntryPoint;
#endif
};

struct PipelineReflection
{
	ShaderStage mShaderStages;
	// the individual stages reflection data.
	ShaderReflection mStageReflections[MAX_SHADER_STAGE_COUNT];
	uint32_t         mStageReflectionCount;

	uint32_t mVertexStageIndex;
	uint32_t mHullStageIndex;
	uint32_t mDomainStageIndex;
	uint32_t mGeometryStageIndex;
	uint32_t mPixelStageIndex;

	ShaderResource* pShaderResources;
	uint32_t        mShaderResourceCount;

	ShaderVariable* pVariables;
	uint32_t        mVariableCount;
};

void destroyShaderReflection(ShaderReflection* pReflection);

void createPipelineReflection(ShaderReflection* pReflection, uint32_t stageCount, PipelineReflection* pOutReflection);
void destroyPipelineReflection(PipelineReflection* pReflection);

// END IShaderReflection
//////////////////////////

typedef enum PrimitiveTopology
{
	PRIMITIVE_TOPO_POINT_LIST = 0,
	PRIMITIVE_TOPO_LINE_LIST,
	PRIMITIVE_TOPO_LINE_STRIP,
	PRIMITIVE_TOPO_TRI_LIST,
	PRIMITIVE_TOPO_TRI_STRIP,
	PRIMITIVE_TOPO_PATCH_LIST,
	PRIMITIVE_TOPO_COUNT,
} PrimitiveTopology;

typedef enum IndexType
{
	INDEX_TYPE_UINT32 = 0,
	INDEX_TYPE_UINT16,
} IndexType;

typedef enum ShaderSemantic
{
	SEMANTIC_UNDEFINED = 0,
	SEMANTIC_POSITION,
	SEMANTIC_NORMAL,
	SEMANTIC_COLOR,
	SEMANTIC_TANGENT,
	SEMANTIC_BITANGENT,
	SEMANTIC_JOINTS,
	SEMANTIC_WEIGHTS,
	SEMANTIC_TEXCOORD0,
	SEMANTIC_TEXCOORD1,
	SEMANTIC_TEXCOORD2,
	SEMANTIC_TEXCOORD3,
	SEMANTIC_TEXCOORD4,
	SEMANTIC_TEXCOORD5,
	SEMANTIC_TEXCOORD6,
	SEMANTIC_TEXCOORD7,
	SEMANTIC_TEXCOORD8,
	SEMANTIC_TEXCOORD9,
} ShaderSemantic;

typedef enum BlendConstant
{
	BC_ZERO = 0,
	BC_ONE,
	BC_SRC_COLOR,
	BC_ONE_MINUS_SRC_COLOR,
	BC_DST_COLOR,
	BC_ONE_MINUS_DST_COLOR,
	BC_SRC_ALPHA,
	BC_ONE_MINUS_SRC_ALPHA,
	BC_DST_ALPHA,
	BC_ONE_MINUS_DST_ALPHA,
	BC_SRC_ALPHA_SATURATE,
	BC_BLEND_FACTOR,
	BC_ONE_MINUS_BLEND_FACTOR,
	MAX_BLEND_CONSTANTS
} BlendConstant;

typedef enum BlendMode
{
	BM_ADD,
	BM_SUBTRACT,
	BM_REVERSE_SUBTRACT,
	BM_MIN,
	BM_MAX,
	MAX_BLEND_MODES,
} BlendMode;

typedef enum CompareMode
{
	CMP_NEVER,
	CMP_LESS,
	CMP_EQUAL,
	CMP_LEQUAL,
	CMP_GREATER,
	CMP_NOTEQUAL,
	CMP_GEQUAL,
	CMP_ALWAYS,
	MAX_COMPARE_MODES,
} CompareMode;

typedef enum StencilOp
{
	STENCIL_OP_KEEP,
	STENCIL_OP_SET_ZERO,
	STENCIL_OP_REPLACE,
	STENCIL_OP_INVERT,
	STENCIL_OP_INCR,
	STENCIL_OP_DECR,
	STENCIL_OP_INCR_SAT,
	STENCIL_OP_DECR_SAT,
	MAX_STENCIL_OPS,
} StencilOp;

static const int RED = 0x1;
static const int GREEN = 0x2;
static const int BLUE = 0x4;
static const int ALPHA = 0x8;
static const int ALL = (RED | GREEN | BLUE | ALPHA);
static const int NONE = 0;

static const int BS_NONE = -1;
static const int DS_NONE = -1;
static const int RS_NONE = -1;

// Blend states are always attached to one of the eight or more render targets that
// are in a MRT
// Mask constants
typedef enum BlendStateTargets
{
	BLEND_STATE_TARGET_0 = 0x1,
	BLEND_STATE_TARGET_1 = 0x2,
	BLEND_STATE_TARGET_2 = 0x4,
	BLEND_STATE_TARGET_3 = 0x8,
	BLEND_STATE_TARGET_4 = 0x10,
	BLEND_STATE_TARGET_5 = 0x20,
	BLEND_STATE_TARGET_6 = 0x40,
	BLEND_STATE_TARGET_7 = 0x80,
	BLEND_STATE_TARGET_ALL = 0xFF,
} BlendStateTargets;
MAKE_ENUM_FLAG(uint32_t, BlendStateTargets)

typedef enum CullMode
{
	CULL_MODE_NONE = 0,
	CULL_MODE_BACK,
	CULL_MODE_FRONT,
	CULL_MODE_BOTH,
	MAX_CULL_MODES
} CullMode;

typedef enum FrontFace
{
	FRONT_FACE_CCW = 0,
	FRONT_FACE_CW
} FrontFace;

typedef enum FillMode
{
	FILL_MODE_SOLID,
	FILL_MODE_WIREFRAME,
	MAX_FILL_MODES
} FillMode;

typedef enum PipelineType
{
	PIPELINE_TYPE_UNDEFINED = 0,
	PIPELINE_TYPE_COMPUTE,
	PIPELINE_TYPE_GRAPHICS,
	PIPELINE_TYPE_RAYTRACING,
	PIPELINE_TYPE_COUNT,
} PipelineType;

typedef enum FilterType
{
	FILTER_NEAREST = 0,
	FILTER_LINEAR,
} FilterType;

typedef enum AddressMode
{
	ADDRESS_MODE_MIRROR,
	ADDRESS_MODE_REPEAT,
	ADDRESS_MODE_CLAMP_TO_EDGE,
	ADDRESS_MODE_CLAMP_TO_BORDER
} AddressMode;

typedef enum MipMapMode
{
	MIPMAP_MODE_NEAREST = 0,
	MIPMAP_MODE_LINEAR
} MipMapMode;

typedef enum DepthStencilClearFlags
{
	ClEAR_DEPTH = 0x01,
	CLEAR_STENCIL = 0x02
} DepthStencilClearFlags;
MAKE_ENUM_FLAG(uint32_t, DepthStencilClearFlags)

typedef union ClearValue
{
	struct
	{
		float r;
		float g;
		float b;
		float a;
	};
	struct
	{
		float  depth;
		uint32 stencil;
	};
} ClearValue;

typedef enum BufferCreationFlags
{
	/// Default flag (Buffer will use aliased memory, buffer will not be cpu accessible until mapBuffer is called)
	BUFFER_CREATION_FLAG_NONE = 0x01,
	/// Buffer will allocate its own memory (COMMITTED resource)
	BUFFER_CREATION_FLAG_OWN_MEMORY_BIT = 0x02,
	/// Buffer will be persistently mapped
	BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT = 0x04,
	/// Use ESRAM to store this buffer
	BUFFER_CREATION_FLAG_ESRAM = 0x08,
	/// Flag to specify not to allocate descriptors for the resource
	BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION = 0x10,

#ifdef METAL
	/* ICB Flags */
	/// Ihnerit pipeline in ICB
	BUFFER_CREATION_FLAG_ICB_INHERIT_PIPELINE = 0x100,
	/// Ihnerit pipeline in ICB
	BUFFER_CREATION_FLAG_ICB_INHERIT_BUFFERS = 0x200,

#endif
} BufferCreationFlags;
MAKE_ENUM_FLAG(uint32_t, BufferCreationFlags)

typedef enum TextureCreationFlags
{
	/// Default flag (Texture will use default allocation strategy decided by the api specific allocator)
	TEXTURE_CREATION_FLAG_NONE = 0,
	/// Texture will allocate its own memory (COMMITTED resource)
	TEXTURE_CREATION_FLAG_OWN_MEMORY_BIT = 0x01,
	/// Texture will be allocated in memory which can be shared among multiple processes
	TEXTURE_CREATION_FLAG_EXPORT_BIT = 0x02,
	/// Texture will be allocated in memory which can be shared among multiple gpus
	TEXTURE_CREATION_FLAG_EXPORT_ADAPTER_BIT = 0x04,
	/// Texture will be imported from a handle created in another process
	TEXTURE_CREATION_FLAG_IMPORT_BIT = 0x08,
	/// Use ESRAM to store this texture
	TEXTURE_CREATION_FLAG_ESRAM = 0x10,
	/// Use on-tile memory to store this texture
	TEXTURE_CREATION_FLAG_ON_TILE = 0x20,
	/// Prevent compression meta data from generating (XBox)
	TEXTURE_CREATION_FLAG_NO_COMPRESSION = 0x40,
	/// Force 2D instead of automatically determining dimension based on width, height, depth
	TEXTURE_CREATION_FLAG_FORCE_2D = 0x80,
	/// Force 3D instead of automatically determining dimension based on width, height, depth
	TEXTURE_CREATION_FLAG_FORCE_3D = 0x100,
	/// Display target
	TEXTURE_CREATION_FLAG_ALLOW_DISPLAY_TARGET = 0x200,
	/// Create an sRGB texture.
	TEXTURE_CREATION_FLAG_SRGB = 0x400,
} TextureCreationFlags;
MAKE_ENUM_FLAG(uint32_t, TextureCreationFlags)

typedef enum GPUPresetLevel
{
	GPU_PRESET_NONE = 0,
	GPU_PRESET_OFFICE,    //This means unsupported
	GPU_PRESET_LOW,
	GPU_PRESET_MEDIUM,
	GPU_PRESET_HIGH,
	GPU_PRESET_ULTRA,
	GPU_PRESET_COUNT
} GPUPresetLevel;

typedef struct BufferBarrier
{
	Buffer*        pBuffer;
	ResourceState  mNewState;
	bool           mSplit;
} BufferBarrier;

typedef struct TextureBarrier
{
	Texture*       pTexture;
	ResourceState  mNewState;
	bool           mSplit;
} TextureBarrier;

typedef struct RenderTargetBarrier
{
	RenderTarget*  pRenderTarget;
	ResourceState  mNewState;
	bool           mSplit;
} RenderTargetBarrier;

typedef struct ReadRange
{
	uint64_t mOffset;
	uint64_t mSize;
} ReadRange;

typedef struct Region3D
{
	uint32_t mXOffset;
	uint32_t mYOffset;
	uint32_t mZOffset;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
} Region3D;

typedef struct Extent3D
{
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
} Extent3D;

typedef enum QueryType
{
	QUERY_TYPE_TIMESTAMP = 0,
	QUERY_TYPE_PIPELINE_STATISTICS,
	QUERY_TYPE_OCCLUSION,
	QUERY_TYPE_COUNT,
} QueryType;

typedef struct QueryPoolDesc
{
	QueryType mType;
	uint32_t  mQueryCount;
	uint32_t  mNodeIndex;
} QueryPoolDesc;

typedef struct QueryDesc
{
	uint32_t mIndex;
} QueryDesc;

typedef struct DEFINE_ALIGNED(QueryPool, 16)
{
#if defined(DIRECT3D12)
	ID3D12QueryHeap* pDxQueryHeap;
	D3D12_QUERY_TYPE mType;
	uint32_t         mCount;
#endif
#if defined(VULKAN)
	VkQueryPool      pVkQueryPool;
	VkQueryType      mType;
	uint32_t         mCount;
#endif
#if defined(DIRECT3D11)
	ID3D11Query**    ppDxQueries;
	D3D11_QUERY      mType;
	uint32_t         mCount;
#endif
#if defined(METAL)
	uint64_t         mGpuTimestampStart;
	uint64_t         mGpuTimestampEnd;
	uint32_t         mCount;
#endif
#if defined(ORBIS)
	OrbisQueryPool   mStruct;
	uint32_t         mType;
	uint32_t         mCount;
#endif
} QueryPool;

/// Data structure holding necessary info to create a Buffer
typedef struct BufferDesc
{
	/// Size of the buffer (in bytes)
	uint64_t mSize;
	/// Alignment
	uint32_t mAlignment;
	/// Decides which memory heap buffer will use (default, upload, readback)
	ResourceMemoryUsage mMemoryUsage;
	/// Creation flags of the buffer
	BufferCreationFlags mFlags;
	/// What state will the buffer get created in
	ResourceState mStartState;
	/// Index of the first element accessible by the SRV/UAV (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
	uint64_t mFirstElement;
	/// Number of elements in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
	uint64_t mElementCount;
	/// Size of each element (in bytes) in the buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
	uint64_t mStructStride;
	/// ICB draw type
	IndirectArgumentType mICBDrawType;
	/// ICB max vertex buffers slots count
	uint32_t mICBMaxVertexBufferBind;
	/// ICB max vertex buffers slots count
	uint32_t mICBMaxFragmentBufferBind;
	/// Set this to specify a counter buffer for this buffer (applicable to BUFFER_USAGE_STORAGE_SRV, BUFFER_USAGE_STORAGE_UAV)
	struct Buffer* pCounterBuffer;
	/// Format of the buffer (applicable to typed storage buffers (Buffer<T>)
	TinyImageFormat m_format;
	/// Flags specifying the suitable usage of this buffer (Uniform buffer, Vertex Buffer, Index Buffer,...)
	DescriptorType mDescriptors;
	/// Debug name used in gpu profile
	const wchar_t* pDebugName;
	uint32_t*      pSharedNodeIndices;
	uint32_t       mNodeIndex;
	uint32_t       mSharedNodeIndexCount;
} BufferDesc;

typedef struct DEFINE_ALIGNED(Buffer, 64)
{
	/// CPU address of the mapped buffer (appliacable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
	void*                            pCpuMappedAddress;
#if defined(DIRECT3D12)
	/// GPU Address - Cache to avoid calls to ID3D12Resource::GetGpuVirtualAddress
	D3D12_GPU_VIRTUAL_ADDRESS        mDxGpuAddress;
	/// Descriptor handle of the CBV in a CPU visible descriptor heap (applicable to BUFFER_USAGE_UNIFORM)
	D3D12_CPU_DESCRIPTOR_HANDLE      mDxDescriptorHandles;
	/// Offset from mDxDescriptors for srv descriptor handle
	uint64_t                         mDxSrvOffset : 8;
	/// Offset from mDxDescriptors for uav descriptor handle
	uint64_t                         mDxUavOffset : 8;
	/// Native handle of the underlying resource
	ID3D12Resource*                  pDxResource;
	/// Contains resource allocation info such as parent heap, offset in heap
	D3D12MA::Allocation*             pDxAllocation;
#endif
#if defined(DIRECT3D11)
	ID3D11Buffer*                    pDxResource;
	ID3D11ShaderResourceView*        pDxSrvHandle;
	ID3D11UnorderedAccessView*       pDxUavHandle;
	uint64_t                         mPadA;
	uint64_t                         mPadB;
#endif
#if defined(VULKAN)
	/// Native handle of the underlying resource
	VkBuffer                         pVkBuffer;
	/// Buffer view
	VkBufferView                     pVkStorageTexelView;
	VkBufferView                     pVkUniformTexelView;
	/// Contains resource allocation info such as parent heap, offset in heap
	VmaAllocation_T*          pVkAllocation;
	uint64_t                         mOffset;
#endif
#if defined(METAL)
	/// Contains resource allocation info such as parent heap, offset in heap
	struct ResourceAllocation*       pMtlAllocation;
	/// Native handle of the underlying resource
	id<MTLBuffer>                    mtlBuffer;
	id<MTLIndirectCommandBuffer>     mtlIndirectCommandBuffer;
	// #TODO: Update Metal Memory Allocator with reference to the new D3D12 Memory Allocator
	uint32_t                         mPositionInHeap;
	uint32_t                         mPadA;
	uint64_t                         mPadB;
#endif
#if defined(ORBIS)
	OrbisBuffer                      mStruct;
#endif
	uint64_t                         mSize : 32;
	uint64_t                         mDescriptors : 20;
	uint64_t                         mMemoryUsage : 3;
	uint64_t                         mNodeIndex : 4;
	/// Current state of the buffer
	uint64_t                         mStartState : 16;
	/// Current state of the buffer
	uint64_t                         mCurrentState : 16;
	/// State of the buffer before mCurrentState (used for state tracking during a split barrier)
	uint64_t                         mPreviousState : 16;
} Buffer;
// One cache line
COMPILE_ASSERT(sizeof(Buffer) == 8 * sizeof(uint64_t));

/// Data structure holding necessary info to create a Texture
typedef struct TextureDesc
{
	/// Texture creation flags (decides memory allocation strategy, sharing access,...)
	TextureCreationFlags mFlags;
	/// Width
	uint32_t m_width;
	/// Height
	uint32_t m_height;
	/// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
	uint32_t m_depth;
	/// Texture array size (Should be 1 if texture is not a texture array or cubemap)
	uint32_t mArraySize;
	/// Number of mip levels
	uint32_t mMipLevels;
	/// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support SAMPLE_COUNT_1)
	SampleCount mSampleCount;
	/// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
	uint32_t mSampleQuality;
	///  image format
	TinyImageFormat m_format;
	/// Optimized clear value (recommended to use this same value when clearing the rendertarget)
	ClearValue mClearValue;
	/// What state will the texture get created in
	ResourceState mStartState;
	/// Descriptor creation
	DescriptorType mDescriptors;
	/// Pointer to native texture handle if the texture does not own underlying resource
	const void* pNativeHandle;
	/// Debug name used in gpu profile
	const wchar_t* pDebugName;
	/// GPU indices to share this texture
	uint32_t* pSharedNodeIndices;
	/// Number of GPUs to share this texture
	uint32_t mSharedNodeIndexCount;
	/// GPU which will own this texture
	uint32_t mNodeIndex;
	/// Is the texture CPU accessible (applicable on hardware supporting CPU mapped textures (UMA))
	bool mHostVisible;
} TextureDesc;

// Virtual texture page as a part of the partially resident texture
// Contains memory bindings, offsets and status information
struct VirtualTexturePage
{
	/// Buffer which contains the image data and be used for copying it to Virtual texture
	Buffer*  pIntermediateBuffer;
	/// Miplevel for this page
	uint32_t mipLevel;
	/// Array layer for this page
	uint32_t layer;
	/// Index for this page
	uint32_t index;
#if defined(DIRECT3D12)
	/// Offset for this page
	D3D12_TILED_RESOURCE_COORDINATE offset;
	/// Size for this page
	D3D12_TILED_RESOURCE_COORDINATE extent;
	/// Byte size for this page
	uint32_t size;
#endif

#if defined(VULKAN)
	/// Offset for this page
	VkOffset3D offset;
	/// Size for this page
	VkExtent3D extent;
	/// Sparse image memory bind for this page
	VkSparseImageMemoryBind imageMemoryBind;
	/// Byte size for this page
	VkDeviceSize size;
#endif
};

typedef struct VirtualTexture
{
#if defined(DIRECT3D12)
	ID3D12Heap* pSparseImageMemory;
	/// Array for Sparse texture's pages
	void* pSparseCoordinates;
	/// Array for heap memory offsets
	void* pHeapRangeStartOffsets;
#endif
#if defined(VULKAN)
	/// Sparse queue binding information
	VkBindSparseInfo mBindSparseInfo;
	/// Sparse image memory bindings of all memory-backed virtual tables
	void* pSparseImageMemoryBinds;
	/// Sparse ?aque memory bindings for the mip tail (if present)	
	void* pOpaqueMemoryBinds;
	/// First mip level in mip tail
	uint32_t mMipTailStart;
	/// Lstly filled mip level in mip tail
	uint32_t mLastFilledMip;
	/// Memory type for Sparse texture's memory
	uint32_t mSparseMemoryTypeIndex;
	/// Sparse image memory bind info 
	VkSparseImageMemoryBindInfo mImageMemoryBindInfo;
	/// Sparse image opaque memory bind info (mip tail)
	VkSparseImageOpaqueMemoryBindInfo mOpaqueMemoryBindInfo;
	/// First mip level in mip tail
	uint32_t mipTailStart;
#endif
	/// Virtual Texture members
	/// Contains all virtual pages of the texture
	void*    pPages;
	/// Visibility data
	Buffer*  mVisibility;
	/// PrevVisibility data
	Buffer*  mPrevVisibility;
	/// Alive Page's Index
	Buffer*  mAlivePage;
	/// Page's Index which should be removed
	Buffer*  mRemovePage;
	/// a { uint32_t alive; uint32_t remove; } count of pages which are alive or should be removed
	Buffer*  mPageCounts;
	/// Original Pixel image data
	void*    mVirtualImageData;
	///  Total pages count
	uint32_t mVirtualPageTotalCount;
	/// Sparse Virtual Texture Width
	uint64_t mSparseVirtualTexturePageWidth;
	/// Sparse Virtual Texture Height
	uint64_t mSparseVirtualTexturePageHeight;
} VirtualTexture;

typedef struct DEFINE_ALIGNED(Texture, 64)
{
#if defined(DIRECT3D12)
	/// Descriptor handle of the SRV in a CPU visible descriptor heap (applicable to TEXTURE_USAGE_SAMPLED_IMAGE)
	D3D12_CPU_DESCRIPTOR_HANDLE  mDxDescriptorHandles;
	/// Native handle of the underlying resource
	ID3D12Resource*              pDxResource;
	/// Contains resource allocation info such as parent heap, offset in heap
	D3D12MA::Allocation*         pDxAllocation;
	uint64_t                     mHandleCount : 24;
	uint64_t                     mDescriptorSize : 6;
	uint64_t                     mPadB;
#endif
#if defined(VULKAN)
	/// Opaque handle used by shaders for doing read/write operations on the texture
	VkImageView                  pVkSRVDescriptor;
	/// Opaque handle used by shaders for doing read/write operations on the texture
	VkImageView*                 pVkUAVDescriptors;
	/// Opaque handle used by shaders for doing read/write operations on the texture
	VkImageView                  pVkSRVStencilDescriptor;
	/// Native handle of the underlying resource
	VkImage                      pVkImage;
	/// Contains resource allocation info such as parent heap, offset in heap
	struct VmaAllocation_T*      pVkAllocation;
#endif
#if defined(METAL)
	/// Contains resource allocation info such as parent heap, offset in heap
	struct ResourceAllocation*   pMtlAllocation;
	/// Native handle of the underlying resource
	id<MTLTexture>               mtlTexture;
	id<MTLTexture> __strong*     pMtlUAVDescriptors;
	id                           mpsTextureAllocator;
	uint64_t                     mtlPixelFormat : 32;
	uint64_t                     mFlags : 31;
	uint64_t                     mIsCompressed : 1;
#endif
#if defined(DIRECT3D11)
	ID3D11Resource*              pDxResource;
	ID3D11ShaderResourceView*    pDxSRVDescriptor;
	ID3D11UnorderedAccessView**  pDxUAVDescriptors;
	uint64_t                     mPadA;
	uint64_t                     mPadB;
#endif
#if defined(ORBIS)
	OrbisTexture                 mStruct;
	/// Contains resource allocation info such as parent heap, offset in heap
#endif
	VirtualTexture*              pSvt;

	/// Current state of the buffer
	uint64_t                     mStartState : 16;
	/// Current state of the buffer
	uint64_t                     mCurrentState : 16;
	/// State of the buffer before mCurrentState (used for state tracking during a split barrier)
	uint64_t                     mPreviousState : 16;
	uint64_t                     mMipLevels : 10;
	uint64_t                     mUav : 1;
	/// This value will be false if the underlying resource is not owned by the texture (swapchain textures,...)
	uint64_t                     mOwnsImage : 1;
	/// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the pVkImageView
	uint64_t                     mAspectMask : 4;
	uint64_t                     mNodeIndex : 4;
	uint64_t                     m_width : 16;
	uint64_t                     m_height : 16;
	uint64_t                     m_depth : 16;
} Texture;
// One cache line
COMPILE_ASSERT(sizeof(Texture) == 8 * sizeof(uint64_t));

typedef struct RenderTargetDesc
{
	/// Texture creation flags (decides memory allocation strategy, sharing access,...)
	TextureCreationFlags mFlags;
	/// Width
	uint32_t m_width;
	/// Height
	uint32_t m_height;
	/// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
	uint32_t m_depth;
	/// Texture array size (Should be 1 if texture is not a texture array or cubemap)
	uint32_t mArraySize;
	/// Number of mip levels
	uint32_t mMipLevels;
	/// MSAA
	SampleCount mSampleCount;
	/// Internal image format
	TinyImageFormat m_format;
	/// Optimized clear value (recommended to use this same value when clearing the rendertarget)
	ClearValue mClearValue;
	/// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
	uint32_t mSampleQuality;
	/// Descriptor creation
	DescriptorType mDescriptors;
	const void*    pNativeHandle;
	/// Debug name used in gpu profile
	const wchar_t* pDebugName;
	/// GPU indices to share this texture
	uint32_t* pSharedNodeIndices;
	/// Number of GPUs to share this texture
	uint32_t mSharedNodeIndexCount;
	/// GPU which will own this texture
	uint32_t mNodeIndex;
} RenderTargetDesc;

typedef struct DEFINE_ALIGNED(RenderTarget, 64)
{
	Texture*                      pTexture;

#if defined(DIRECT3D12)
	D3D12_CPU_DESCRIPTOR_HANDLE   mDxDescriptors;
	uint32_t                      mDxDescriptorSize : 8;
	uint32_t                      mPadA;
	uint64_t                      mPadB;
#endif
#if defined(VULKAN)
	VkImageView                   pVkDescriptor;
	VkImageView*                  pVkSliceDescriptors;
	uint64_t                      mId;
#endif
#if defined(METAL)
	uint64_t                      mPadA[3];
#endif
#if defined(DIRECT3D11)
	union
	{
		/// Resources
		ID3D11RenderTargetView*  pDxRtvDescriptor;
		ID3D11DepthStencilView*  pDxDsvDescriptor;
	};
	union
	{
		/// Resources
		ID3D11RenderTargetView** pDxRtvSliceDescriptors;
		ID3D11DepthStencilView** pDxDsvSliceDescriptors;
	};
	uint64_t                      mPadA;
#endif
#if defined(ORBIS)
	OrbisRenderTarget             mStruct;
#endif
	ClearValue                    mClearValue;
	uint32_t                      mArraySize : 16;
	uint32_t                      m_depth : 16;
	uint32_t                      m_width : 16;
	uint32_t                      m_height : 16;
	uint32_t                      mDescriptors : 20;
	uint32_t                      mMipLevels : 10;
	uint32_t                      mSampleQuality : 5;
	TinyImageFormat               m_format;
	SampleCount                   mSampleCount;
} RenderTarget;
// Two cache lines
COMPILE_ASSERT(sizeof(RenderTarget) == 16 * sizeof(uint64_t));

typedef struct LoadActionsDesc
{
	ClearValue     mClearColorValues[MAX_RENDER_TARGET_ATTACHMENTS];
	LoadActionType mLoadActionsColor[MAX_RENDER_TARGET_ATTACHMENTS];
	ClearValue     mClearDepth;
	LoadActionType mLoadActionDepth;
	LoadActionType mLoadActionStencil;
} LoadActionsDesc;

typedef struct SamplerDesc
{
	FilterType  mMinFilter;
	FilterType  mMagFilter;
	MipMapMode  mMipMapMode;
	AddressMode mAddressU;
	AddressMode mAddressV;
	AddressMode mAddressW;
	float       mMipLodBias;
	float       mMaxAnisotropy;
	CompareMode mCompareFunc;
} SamplerDesc;

typedef struct DEFINE_ALIGNED(Sampler, 16)
{
#if defined(DIRECT3D12)
	/// Description for creating the Sampler descriptor for ths sampler
	D3D12_SAMPLER_DESC          mDxDesc;
	/// Descriptor handle of the Sampler in a CPU visible descriptor heap
	D3D12_CPU_DESCRIPTOR_HANDLE mDxHandle;
#endif
#if defined(VULKAN)
	/// Native handle of the underlying resource
	VkSampler                   pVkSampler;
#endif
#if defined(METAL)
	/// Native handle of the underlying resource
	id<MTLSamplerState>         mtlSamplerState;
#endif
#if defined(DIRECT3D11)
	/// Native handle of the underlying resource
	ID3D11SamplerState*         pSamplerState;
#endif
#if defined(ORBIS)
	OrbisSampler                mStruct;
#endif
} Sampler;
#if defined(DIRECT3D12)
COMPILE_ASSERT(sizeof(Sampler) == 8 * sizeof(uint64_t));
#else
COMPILE_ASSERT(sizeof(Sampler) == 2 * sizeof(uint64_t));
#endif

typedef enum DescriptorUpdateFrequency
{
	DESCRIPTOR_UPDATE_FREQ_NONE = 0,
	DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
	DESCRIPTOR_UPDATE_FREQ_PER_BATCH,
	DESCRIPTOR_UPDATE_FREQ_PER_DRAW,
	DESCRIPTOR_UPDATE_FREQ_COUNT,
} DescriptorUpdateFrequency;

/// Data structure holding the layout for a descriptor
typedef struct DEFINE_ALIGNED(DescriptorInfo, 16)
{
#if defined(ORBIS)
	OrbisDescriptorInfo       mStruct;
#else
	const char*               pName;
	uint32_t                  mType : 20;
	uint32_t                  mDim : 4;
	uint32_t                  mRootDescriptor : 1;
	uint32_t                  mUpdateFrequency : 3;
	uint32_t                  mSize;
	/// Index in the descriptor set
	uint32_t                  mIndexInParent;
	uint32_t                  mHandleIndex;
#if defined(VULKAN)
	uint32_t                  mVkType;
	uint32_t                  mReg : 20;
	uint32_t                  mRootDescriptorIndex : 3;
	uint32_t                  mVkStages : 8;
#elif defined(DIRECT3D11)
	uint32_t                  mUsedStages : 6;
	uint32_t                  mReg : 20;
	uint32_t                  mPadA;
#elif defined(DIRECT3D12)
	uint64_t                  mPadA;
#endif
#if defined(METAL)
	id<MTLSamplerState>       mtlStaticSampler;
	const ArgumentDescriptor* mtlArgumentDescriptors;
	uint32_t                  mUsedStages : 6;
	uint32_t                  mReg : 10;
	uint32_t                  mIsArgumentBufferField : 1;
	uint32_t                  mPadA;
	uint64_t                  mPadB[2];
#endif
#endif
} DescriptorInfo;
#if defined(METAL)
COMPILE_ASSERT(sizeof(DescriptorInfo) == 8 * sizeof(uint64_t));
#elif defined(ORBIS)
COMPILE_ASSERT(sizeof(DescriptorInfo) == 2 * sizeof(uint64_t));
#else
COMPILE_ASSERT(sizeof(DescriptorInfo) == 4 * sizeof(uint64_t));
#endif

typedef enum RootSignatureFlags
{
	/// Default flag
	ROOT_SIGNATURE_FLAG_NONE = 0,
	/// Local root signature used mainly in raytracing shaders
	ROOT_SIGNATURE_FLAG_LOCAL_BIT = 0x1,
} RootSignatureFlags;
MAKE_ENUM_FLAG(uint32_t, RootSignatureFlags)

typedef struct RootSignatureDesc
{
	Shader**               ppShaders;
	uint32_t               mShaderCount;
	uint32_t               mMaxBindlessTextures;
	const char**           ppStaticSamplerNames;
	Sampler**              ppStaticSamplers;
	uint32_t               mStaticSamplerCount;
	RootSignatureFlags     mFlags;
} RootSignatureDesc;

typedef struct DEFINE_ALIGNED(RootSignature, 64)
{
	/// Number of descriptors declared in the root signature layout
	uint32_t                   mDescriptorCount;
	/// Graphics or Compute
	PipelineType               mPipelineType;
	/// Array of all descriptors declared in the root signature layout
	DescriptorInfo*            pDescriptors;
	/// Translates hash of descriptor name to descriptor index in pDescriptors array
	DescriptorIndexMap*        pDescriptorNameToIndexMap;
#if defined(DIRECT3D12)
	ID3D12RootSignature*       pDxRootSignature;
	uint8_t                    mDxRootConstantRootIndices[MAX_ROOT_CONSTANTS_PER_ROOTSIGNATURE];
	uint8_t                    mDxViewDescriptorTableRootIndices[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint8_t                    mDxSamplerDescriptorTableRootIndices[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint8_t                    mDxRootDescriptorRootIndices[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint32_t                   mDxCumulativeViewDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint32_t                   mDxCumulativeSamplerDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint16_t                   mDxViewDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint16_t                   mDxSamplerDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint8_t                    mDxRootDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint32_t                   mDxRootConstantCount;
	ID3DBlob*                  pDxSerializedRootSignatureString;
	uint64_t                   mPadA;
	uint64_t                   mPadB;
#endif
#if defined(VULKAN)
	VkDescriptorSetLayout      mVkDescriptorSetLayouts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint32_t                   mVkCumulativeDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint16_t                   mVkDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint8_t                    mVkDynamicDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint8_t                    mVkRaytracingDescriptorCounts[DESCRIPTOR_UPDATE_FREQ_COUNT];
	VkPipelineLayout           pPipelineLayout;
	VkDescriptorUpdateTemplate mUpdateTemplates[DESCRIPTOR_UPDATE_FREQ_COUNT];
	VkDescriptorSet            mVkEmptyDescriptorSets[DESCRIPTOR_UPDATE_FREQ_COUNT];
	void**                     pUpdateTemplateData[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint32_t                   mVkPushConstantCount;
	uint32_t                   mPadA;
	uint64_t                   mPadB[7];
#endif
#if defined(METAL)
	// paramIndex support
	typedef struct IndexedDescriptor
	{
		const DescriptorInfo** pDescriptors;
		uint32_t               mDescriptorCount;
	} IndexedDescriptor;

	ShaderDescriptors*         pShaderDescriptors;
	IndexedDescriptor*         mIndexedDescriptorInfo;
	uint32_t                   mShaderDescriptorsCount;
	uint32_t                   mIndexedDescriptorCount;
	uint64_t                   mPadA[2];
#endif
#if defined(DIRECT3D11)
	ID3D11SamplerState**       ppStaticSamplers;
	uint32_t*                  pStaticSamplerSlots;
	ShaderStage*               pStaticSamplerStages;
	uint32_t                   mStaticSamplerCount;
	uint32_t                   mSrvCount : 10;
	uint32_t                   mUavCount : 10;
	uint32_t                   mCbvCount : 10;
	uint32_t                   mSamplerCount : 10;
	uint32_t                   mDynamicCbvCount : 10;
	uint32_t                   mPadA;
#endif
#if defined(ORBIS)
	OrbisRootSignature         mStruct;
#endif
} RootSignature;
#if defined(VULKAN)
// 4 cache lines
COMPILE_ASSERT(sizeof(RootSignature) == 32 * sizeof(uint64_t));
#elif defined(DIRECT3D11) || defined(METAL)
// 1 cache line
COMPILE_ASSERT(sizeof(RootSignature) == 8 * sizeof(uint64_t));
#else
// 2 cache lines
COMPILE_ASSERT(sizeof(RootSignature) == 16 * sizeof(uint64_t));
#endif

typedef struct DescriptorData
{
	/// User can either set name of descriptor or index (index in pRootSignature->pDescriptors array)
	/// Name of descriptor
	const char* pName;
	union
	{
		struct
		{
			/// Offset to bind the buffer descriptor
			const uint64_t* pOffsets;
			const uint64_t* pSizes;
		};

		// Descriptor set buffer extraction options
		struct
		{
			uint32_t    mDescriptorSetBufferIndex;
			Shader*     mDescriptorSetShader;
			ShaderStage mDescriptorSetShaderStage;
		};

		uint32_t mUAVMipSlice;
		bool mBindStencilResource;
	};
	/// Array of resources containing descriptor handles or constant to be used in ring buffer memory - DescriptorRange can hold only one resource type array
	union
	{
		/// Array of texture descriptors (srv and uav textures)
		Texture** ppTextures;
		/// Array of sampler descriptors
		Sampler** ppSamplers;
		/// Array of buffer descriptors (srv, uav and cbv buffers)
		Buffer** ppBuffers;
		/// Array of pipline descriptors
		Pipeline** ppPipelines;
		/// DescriptorSet buffer extraction
		DescriptorSet** ppDescriptorSet;
		/// Custom binding (raytracing acceleration structure ...)
		AccelerationStructure** ppAccelerationStructures;
	};
	/// Number of resources in the descriptor(applies to array of textures, buffers,...)
	uint32_t mCount;
	uint32_t mIndex = (uint32_t)-1;
	bool     mExtractBuffer = false;
} DescriptorData;

typedef struct DEFINE_ALIGNED(DescriptorSet, 64)
{
#if defined(DIRECT3D12)
	/// Start handle to cbv srv uav descriptor table
	uint64_t                      mCbvSrvUavHandle;
	/// Start handle to sampler descriptor table
	uint64_t                      mSamplerHandle;
	/// Stride of the cbv srv uav descriptor table (number of descriptors * descriptor size)
	uint32_t                      mCbvSrvUavStride;
	/// Stride of the sampler descriptor table (number of descriptors * descriptor size)
	uint32_t                      mSamplerStride;
	const RootSignature*          pRootSignature;
	D3D12_GPU_VIRTUAL_ADDRESS*    pRootAddresses;
	ID3D12RootSignature*          pRootSignatureHandle;
	uint64_t                      mMaxSets : 16;
	uint64_t                      mUpdateFrequency : 3;
	uint64_t                      mNodeIndex : 4;
	uint64_t                      mRootAddressCount : 1;
	uint64_t                      mCbvSrvUavRootIndex : 4;
	uint64_t                      mSamplerRootIndex : 4;
	uint64_t                      mRootDescriptorRootIndex : 4;
	uint64_t                      mPipelineType : 3;
#elif defined(VULKAN)
	VkDescriptorSet*              pHandles;
	const RootSignature*          pRootSignature;
	/// Values passed to vkUpdateDescriptorSetWithTemplate. Initialized to default descriptor values.
	union DescriptorUpdateData**  ppUpdateData;
	struct SizeOffset*            pDynamicSizeOffsets;
	uint32_t                      mMaxSets;
	uint8_t                       mDynamicOffsetCount;
	uint8_t                       mUpdateFrequency;
	uint8_t                       mNodeIndex;
	uint8_t                       mPadA;
#elif defined(METAL)
	struct MTLDescriptorSet*      pMtl;
#elif defined(DIRECT3D11)
	struct DescriptorDataArray*   pHandles;
	struct CBV**                  pDynamicCBVs;
	uint32_t*                     pDynamicCBVsCapacity;
	uint32_t*                     pDynamicCBVsCount;
	uint32_t*                     pDynamicCBVsPrevCount;
	const RootSignature*          pRootSignature;
	uint16_t                      mMaxSets;
#elif defined(ORBIS)
	OrbisDescriptorSet            mStruct;
#endif
} DescriptorSet;

typedef struct CmdPoolDesc
{
	Queue*      pQueue;
	bool        mTransient;
} CmdPoolDesc;

typedef struct DEFINE_ALIGNED(CmdPool, 16)
{
#if defined(DIRECT3D12)
	// Temporarily move to Cmd struct until we get the command allocator pool logic working
	//ID3D12CommandAllocator*	   pDxCmdAlloc;
#endif
#if defined(VULKAN)
	VkCommandPool pVkCmdPool;
#endif
	Queue*        pQueue;
} CmdPool;

typedef struct CmdDesc
{
	CmdPool* pPool;
#if defined(ORBIS)
	uint32_t mMaxSize;
#endif
	bool     mSecondary;
} CmdDesc;

typedef struct DEFINE_ALIGNED(Cmd, 64)
{
#if defined(DIRECT3D12)
#if defined(_DURANGO)
	DmaCmd                       mDma;
#endif
	// For now each command list will have its own allocator until we get the command allocator pool logic working
	ID3D12CommandAllocator*      pDxCmdAlloc;
	ID3D12GraphicsCommandList*   pDxCmdList;

	// Cached in beginCmd to avoid fetching them during rendering
	struct DescriptorHeap*       pBoundHeaps[2];
	D3D12_GPU_DESCRIPTOR_HANDLE  mBoundHeapStartHandles[2];

	// Command buffer state
	const ID3D12RootSignature*   pBoundRootSignature;
	DescriptorSet*               pBoundDescriptorSets[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint16_t                     mBoundDescriptorSetIndices[DESCRIPTOR_UPDATE_FREQ_COUNT];
	uint32_t                     mNodeIndex : 4;
	uint32_t                     mType : 3;
	uint32_t                     mPadA;
#if !defined(_DURANGO)
	uint64_t                     mPadB;
#endif
#endif
#if defined(VULKAN)
	VkCommandBuffer              pVkCmdBuf;
	VkRenderPass                 pVkActiveRenderPass;
	VkPipelineLayout             pBoundPipelineLayout;
	uint32_t                     mNodeIndex : 4;
	uint32_t                     mType : 3;
	uint32_t                     mPadA;
	CmdPool*                     pCmdPool;
	uint64_t                     mPadB[9];
#endif
#if defined(METAL)
	id<MTLCommandBuffer>         mtlCommandBuffer;
	id<MTLRenderCommandEncoder>  mtlRenderEncoder;
	id<MTLComputeCommandEncoder> mtlComputeEncoder;
	id<MTLBlitCommandEncoder>    mtlBlitEncoder;
	MTLRenderPassDescriptor*     pRenderPassDesc;
	Shader*                      pShader;
	Buffer*                      selectedIndexBuffer;
	uint64_t                     mSelectedIndexBufferOffset : 32;
	uint64_t                     mIndexType : 1;
	uint64_t                     mIndexStride : 3;
	uint64_t                     selectedPrimitiveType : 4;
	QueryPool*                   pLastFrameQuery;
	uint64_t                     mPadA[5];
#endif
#if defined(DIRECT3D11)
	ID3D11Buffer*                pRootConstantBuffer;
	ID3D11Buffer*                pTransientConstantBuffer;
	uint8_t*                     pDescriptorCache;
	uint32_t                     mDescriptorCacheOffset;
	uint32_t                     mPadA;
	uint64_t                     mPadB[10];
#endif
#if defined(ORBIS)
	OrbisCmd                     mStruct;
#endif
	Renderer*                    pRenderer;
	Queue*                       pQueue;
} Cmd;
COMPILE_ASSERT(sizeof(Cmd) <= 64 * sizeof(uint64_t));

typedef enum FenceStatus
{
	FENCE_STATUS_COMPLETE = 0,
	FENCE_STATUS_INCOMPLETE,
	FENCE_STATUS_NOTSUBMITTED,
} FenceStatus;

typedef struct DEFINE_ALIGNED(Fence, 32)
{
#if defined(DIRECT3D12)
	ID3D12Fence*         pDxFence;
	HANDLE               pDxWaitIdleFenceEvent;
	uint64_t             mFenceValue;
	uint64_t             mPadA;
#endif
#if defined(VULKAN)
	VkFence              pVkFence;
	uint32_t             mSubmitted : 1;
	uint32_t             mPadA;
	uint64_t             mPadB;
	uint64_t             mPadC;
#endif
#if defined(METAL)
	dispatch_semaphore_t pMtlSemaphore;
	uint32_t             mSubmitted : 1;
	uint32_t             mPadA;
	uint64_t             mPadB;
	uint64_t             mPadC;
#endif
#if defined(DIRECT3D11)
	uint64_t             mPadA;
	uint64_t             mPadB;
	uint64_t             mPadC;
	uint64_t             mPadD;
#endif
#if defined(ORBIS)
	OrbisFence           mStruct;
#endif
} Fence;
COMPILE_ASSERT(sizeof(Fence) == 4 * sizeof(uint64_t));

typedef struct DEFINE_ALIGNED(Semaphore, 32)
{
#if defined(DIRECT3D12)
	// DirectX12 does not have a concept of semaphores
	// All synchronization is done using fences
	// Simlate semaphore signal and wait using DirectX12 fences

	// Semaphores used in DirectX12 only in queueSubmit
	// queueSubmit -> How the semaphores work in DirectX12

	// pp_wait_semaphores -> queue->Wait is manually called on each fence in this
	// array before calling ExecuteCommandLists to make the fence work like a wait semaphore

	// pp_signal_semaphores -> Manually call queue->Signal on each fence in this array after
	// calling ExecuteCommandLists and increment the underlying fence value

	// queuePresent does not use the wait semaphore since the swapchain Present function
	// already does the synchronization in this case
	ID3D12Fence*         pDxFence;
	HANDLE               pDxWaitIdleFenceEvent;
	uint64_t             mFenceValue;
	uint64_t             mPadA;
#endif
#if defined(VULKAN)
	VkSemaphore          pVkSemaphore;
	uint32_t             mCurrentNodeIndex : 5;
	uint32_t             mSignaled : 1;
	uint32_t             mPadA;
	uint64_t             mPadB;
	uint64_t             mPadC;
#endif
#if defined(METAL)
	dispatch_semaphore_t pMtlSemaphore;
	uint64_t             mPadA;
	uint64_t             mPadB;
	uint64_t             mPadC;
#endif
#if defined(ORBIS)
	OrbisFence           mStruct;
#endif
} Semaphore;
COMPILE_ASSERT(sizeof(Semaphore) == 4 * sizeof(uint64_t));

typedef struct QueueDesc
{
	QueueType     mType;
	QueueFlag     mFlag;
	QueuePriority mPriority;
	uint32_t      mNodeIndex;
} QueueDesc;

typedef struct DEFINE_ALIGNED(Queue, 64)
{
#if defined(DIRECT3D12)
	ID3D12CommandQueue*  pDxQueue;
	uint32_t             mType : 3;
	uint32_t             mNodeIndex : 4;
	Extent3D             mUploadGranularity;
	Fence*               pFence;
#endif
#if defined(VULKAN)
	VkQueue              pVkQueue;
	uint32_t             mVkQueueFamilyIndex : 5;
	uint32_t             mVkQueueIndex : 5;
	uint32_t             mType : 3;
	uint32_t             mNodeIndex : 4;
	uint32_t             mGpuMode : 3;
	uint32_t             mFlags;
	float                mTimestampPeriod;
	Extent3D             mUploadGranularity;
#endif
#if defined(METAL)
	id<MTLCommandQueue>  mtlCommandQueue;
	id<MTLFence>         mtlQueueFence;
	uint32_t             mBarrierFlags;
	uint32_t             mType : 3;
	uint32_t             mNodeIndex : 4;
	uint32_t             mPadB;
	Extent3D             mUploadGranularity;
#endif
#if defined(DIRECT3D11)
	ID3D11DeviceContext* pDxContext;
	uint32_t             mType : 3;
	uint32_t             mNodeIndex : 4;
	Extent3D             mUploadGranularity;
#endif
#if defined(ORBIS)
	OrbisQueue           mStruct;
	uint32_t             mType : 3;
	uint32_t             mNodeIndex : 4;
	Extent3D             mUploadGranularity;
#endif
} Queue;
COMPILE_ASSERT(sizeof(Queue) <= 32 * sizeof(uint64_t));

typedef struct ShaderMacro
{
	const char* definition;
	const char* value;
} ShaderMacro;

#if defined(TARGET_IOS)
typedef struct ShaderStageDesc
{
	const char*  pName;
	const char*  pCode;
	const char*  pEntryPoint;
	ShaderMacro* pMacros;
	uint32_t     mMacroCount;
} ShaderStageDesc;

typedef struct ShaderDesc
{
	ShaderStage     mStages;
	ShaderStageDesc mVert;
	ShaderStageDesc mFrag;
	ShaderStageDesc mGeom;
	ShaderStageDesc mHull;
	ShaderStageDesc mDomain;
	ShaderStageDesc mComp;
} ShaderDesc;
#endif

typedef struct BinaryShaderStageDesc
{
	/// Byte code array
	const char*    pByteCode;
	uint32_t       mByteCodeSize;
	const char*    pEntryPoint;
#if defined(METAL)
	// Shader source is needed for reflection
	char*          pSource;
	uint32_t       mSourceSize;
#endif
} BinaryShaderStageDesc;

typedef struct BinaryShaderDesc
{
	ShaderStage           mStages;
	BinaryShaderStageDesc mVert;
	BinaryShaderStageDesc mFrag;
	BinaryShaderStageDesc mGeom;
	BinaryShaderStageDesc mHull;
	BinaryShaderStageDesc mDomain;
	BinaryShaderStageDesc mComp;
} BinaryShaderDesc;

typedef struct DEFINE_ALIGNED(Shader, 64)
{
#if defined(DIRECT3D12)
	ID3DBlob**                    pShaderBlobs;
	LPCWSTR*                      pEntryNames;
#endif
#if defined(VULKAN)
	VkShaderModule*               pShaderModules;
	char**                        pEntryNames;
#endif
#if defined(METAL)
	id<MTLFunction>               mtlVertexShader;
	id<MTLFunction>               mtlFragmentShader;
	id<MTLFunction>               mtlComputeShader;
	id<MTLLibrary>		          mtlLibrary;
	char**                        pEntryNames;
#endif
#if defined(DIRECT3D11)
	union
	{
		struct
		{
			ID3D11VertexShader*   pDxVertexShader;
			ID3D11PixelShader*    pDxPixelShader;
			ID3D11GeometryShader* pDxGeometryShader;
			ID3D11DomainShader*   pDxDomainShader;
			ID3D11HullShader*     pDxHullShader;
		};
		ID3D11ComputeShader*      pDxComputeShader;
	};
	ID3DBlob*                     pDxInputSignature;
#endif
#if defined(ORBIS)
	OrbisShader                   mStruct;
#endif
	PipelineReflection*           pReflection;
	ShaderStage                   mStages;
	uint32_t                      mNumThreadsPerGroup[3];
} Shader;

typedef struct BlendStateDesc
{
	/// Source blend factor per render target.
	BlendConstant mSrcFactors[MAX_RENDER_TARGET_ATTACHMENTS];
	/// Destination blend factor per render target.
	BlendConstant mDstFactors[MAX_RENDER_TARGET_ATTACHMENTS];
	/// Source alpha blend factor per render target.
	BlendConstant mSrcAlphaFactors[MAX_RENDER_TARGET_ATTACHMENTS];
	/// Destination alpha blend factor per render target.
	BlendConstant mDstAlphaFactors[MAX_RENDER_TARGET_ATTACHMENTS];
	/// Blend mode per render target.
	BlendMode mBlendModes[MAX_RENDER_TARGET_ATTACHMENTS];
	/// Alpha blend mode per render target.
	BlendMode mBlendAlphaModes[MAX_RENDER_TARGET_ATTACHMENTS];
	/// Write mask per render target.
	int32_t mMasks[MAX_RENDER_TARGET_ATTACHMENTS];
	/// Mask that identifies the render targets affected by the blend state.
	BlendStateTargets mRenderTargetMask;
	/// Set whether alpha to coverage should be enabled.
	bool mAlphaToCoverage;
	/// Set whether each render target has an unique blend function. When false the blend function in slot 0 will be used for all render targets.
	bool mIndependentBlend;
} BlendStateDesc;

typedef struct DepthStateDesc
{
	bool        mDepthTest;
	bool        mDepthWrite;
	CompareMode mDepthFunc = CompareMode::CMP_LEQUAL;
	bool        mStencilTest;
	uint8_t     mStencilReadMask;
	uint8_t     mStencilWriteMask;
	CompareMode mStencilFrontFunc = CompareMode::CMP_ALWAYS;
	StencilOp   mStencilFrontFail;
	StencilOp   mDepthFrontFail;
	StencilOp   mStencilFrontPass;
	CompareMode mStencilBackFunc = CompareMode::CMP_ALWAYS;
	StencilOp   mStencilBackFail;
	StencilOp   mDepthBackFail;
	StencilOp   mStencilBackPass;
} DepthStateDesc;

typedef struct RasterizerStateDesc
{
	CullMode  mCullMode;
	int32_t   mDepthBias;
	float     mSlopeScaledDepthBias;
	FillMode  mFillMode;
	bool      mMultiSample;
	bool      mScissor;
	FrontFace mFrontFace;
	bool      mDepthClampEnable;
} RasterizerStateDesc;

typedef enum VertexAttribRate
{
	VERTEX_ATTRIB_RATE_VERTEX = 0,
	VERTEX_ATTRIB_RATE_INSTANCE = 1,
	VERTEX_ATTRIB_RATE_COUNT,
} VertexAttribRate;

typedef struct VertexAttrib
{
	ShaderSemantic    mSemantic;
	uint32_t          mSemanticNameLength;
	char              mSemanticName[MAX_SEMANTIC_NAME_LENGTH];
	TinyImageFormat	  m_format;
	uint32_t          mBinding;
	uint32_t          mLocation;
	uint32_t          mOffset;
	VertexAttribRate  mRate;

} VertexAttrib;

typedef struct VertexLayout
{
	uint32_t     mAttribCount;
	VertexAttrib mAttribs[MAX_VERTEX_ATTRIBS];
} VertexLayout;

/************************************************************************/
// #pGlobalRootSignature - Root Signature used by all shaders in the ppShaders array
// #ppShaders - Array of all shaders which can be called during the raytracing operation
//	  This includes the ray generation shader, all miss, any hit, closest hit shaders
// #pHitGroups - Name of the hit groups which will tell the pipeline about which combination of hit shaders to use
// #mPayloadSize - Size of the payload struct for passing data to and from the shaders.
//	  Example - float4 payload sent by raygen shader which will be filled by miss shader as a skybox color
//				  or by hit shader as shaded color
// #mAttributeSize - Size of the intersection attribute. As long as user uses the default intersection shader
//	  this size is sizeof(float2) which represents the ZW of the barycentric co-ordinates of the intersection
/************************************************************************/
typedef struct RaytracingPipelineDesc
{
	Raytracing*			pRaytracing;
	RootSignature*		pGlobalRootSignature;
	Shader*             pRayGenShader;
	RootSignature*		pRayGenRootSignature;
	Shader**            ppMissShaders;
	RootSignature**		ppMissRootSignatures;
	RaytracingHitGroup* pHitGroups;
	RootSignature*		pEmptyRootSignature;
	unsigned			mMissShaderCount;
	unsigned			mHitGroupCount;
	// #TODO : Remove this after adding shader reflection for raytracing shaders
	unsigned			mPayloadSize;
	// #TODO : Remove this after adding shader reflection for raytracing shaders
	unsigned			mAttributeSize;
	unsigned			mMaxTraceRecursionDepth;
	unsigned            mMaxRaysCount;
} RaytracingPipelineDesc;


typedef struct GraphicsPipelineDesc
{
	Shader*                pShaderProgram;
	RootSignature*         pRootSignature;
	VertexLayout*          pVertexLayout;
	BlendStateDesc*        pBlendState;
	DepthStateDesc*        pDepthState;
	RasterizerStateDesc*   pRasterizerState;
	TinyImageFormat* 	   pColorFormats;
	uint32_t               mRenderTargetCount;
	SampleCount            mSampleCount;
	uint32_t               mSampleQuality;
	TinyImageFormat  	   mDepthStencilFormat;
	PrimitiveTopology      mPrimitiveTopo;
	bool                   mSupportIndirectCommandBuffer;
} GraphicsPipelineDesc;

typedef struct ComputePipelineDesc
{
	Shader*        pShaderProgram;
	RootSignature* pRootSignature;
} ComputePipelineDesc;

typedef struct PipelineDesc
{
	PipelineType mType;
	union
	{
		ComputePipelineDesc		mComputeDesc;
		GraphicsPipelineDesc	mGraphicsDesc;
		RaytracingPipelineDesc	mRaytracingDesc;
	};
} PipelineDesc;

#ifdef METAL
typedef struct RaytracingPipeline RaytracingPipeline;
#endif

typedef struct DEFINE_ALIGNED(Pipeline, 64)
{
#if defined(DIRECT3D12)
	ID3D12PipelineState*        pDxPipelineState;
#ifdef ENABLE_RAYTRACING
	ID3D12StateObject*	        pDxrPipeline;
#endif
	ID3D12RootSignature*        pRootSignature;
	PipelineType                mType;
	D3D_PRIMITIVE_TOPOLOGY      mDxPrimitiveTopology;
	uint64_t                    mPadB[3];
#endif
#if defined(VULKAN)
	VkPipeline                  pVkPipeline;
	PipelineType                mType;
	uint32_t                    mShaderStageCount;
	//In DX12 this information is stored in ID3D12StateObject.
	//But for Vulkan we need to store it manually
	const char**                ppShaderStageNames;
	uint64_t                    mPadB[4];
#endif
#if defined(METAL)
	Shader*                     pShader;
	id<MTLRenderPipelineState>  mtlRenderPipelineState;
	id<MTLComputePipelineState> mtlComputePipelineState;
	id<MTLDepthStencilState>    mtlDepthStencilState;
	RaytracingPipeline*         pRaytracingPipeline;
	uint32_t                    mCullMode : 3;
	uint32_t                    mFillMode : 3;
	uint32_t                    mWinding : 3;
	uint32_t                    mMtlPrimitiveType : 4;
	float                       mDepthBias;
	float                       mSlopeScale;
	PipelineType                mType;
	uint64_t                    mPadA;
#endif
#if defined(DIRECT3D11)
	ID3D11VertexShader*         pDxVertexShader;
	ID3D11PixelShader*          pDxPixelShader;
	ID3D11GeometryShader*       pDxGeometryShader;
	ID3D11DomainShader*         pDxDomainShader;
	ID3D11HullShader*           pDxHullShader;
	ID3D11ComputeShader*        pDxComputeShader;
	ID3D11InputLayout*          pDxInputLayout;
	ID3D11BlendState*           pBlendState;
	ID3D11DepthStencilState*    pDepthState;
	ID3D11RasterizerState*      pRasterizerState;
	PipelineType                mType;
	D3D_PRIMITIVE_TOPOLOGY      mDxPrimitiveTopology;
	uint32_t                    mPadA;
	uint64_t                    mPadB[4];
#endif
#if defined(ORBIS)
	OrbisPipeline               mStruct;
#endif
} Pipeline;
#if defined(DIRECT3D11) || defined(ORBIS)
// Requires more cache lines due to no concept of an encapsulated pipeline state object
COMPILE_ASSERT(sizeof(Pipeline) <= 64 * sizeof(uint64_t));
#else
// One cache line
COMPILE_ASSERT(sizeof(Pipeline) == 8 * sizeof(uint64_t));
#endif

typedef struct SubresourceDataDesc
{
	// Source description
	uint64_t mBufferOffset;
	uint32_t mRowPitch;
	uint32_t mSlicePitch;
	// Destination description
	uint32_t mArrayLayer;
	uint32_t mMipLevel;
	Region3D mRegion;
} SubresourceDataDesc;

typedef struct SwapChainDesc
{
	/// Window handle
	WindowHandle mWindowHandle;
	/// Queues which should be allowed to present
	Queue** ppPresentQueues;
	/// Number of present queues
	uint32_t mPresentQueueCount;
	/// Number of backbuffers in this swapchain
	uint32_t mImageCount;
	/// Width of the swapchain
	uint32_t m_width;
	/// Height of the swapchain
	uint32_t m_height;
	/// Color format of the swapchain
	TinyImageFormat mColorFormat;
	/// Clear value
	ClearValue mColorClearValue;
	/// Set whether swap chain will be presented using vsync
	bool mEnableVsync;
} SwapChainDesc;

typedef struct DEFINE_ALIGNED(SwapChain, 64)
{
	/// Render targets created from the swapchain back buffers
	RenderTarget**           ppRenderTargets;
#if defined(_DURANGO)
	IDXGISwapChain1*         pDxSwapChain;
	/// Sync interval to specify how interval for vsync
	uint32_t                 mDxSyncInterval : 3;
	uint32_t                 mFlags : 10;
	uint32_t                 mImageCount : 3;
	uint32_t                 mEnableVsync : 1;
	uint32_t                 mIndex;
	void*                    pWindow;
	uint64_t                 mPadB[4];
#elif defined(DIRECT3D12)
	/// Use IDXGISwapChain3 for now since IDXGISwapChain4
	/// isn't supported by older devices.
	IDXGISwapChain3*         pDxSwapChain;
	/// Sync interval to specify how interval for vsync
	uint32_t                 mDxSyncInterval : 3;
	uint32_t                 mFlags : 10;
	uint32_t                 mImageCount : 3;
	uint32_t                 mEnableVsync : 1;
	uint32_t                 mPadA;
	uint64_t                 mPadB[5];
#endif
#if defined(DIRECT3D11)
	/// Use IDXGISwapChain3 for now since IDXGISwapChain4
	/// isn't supported by older devices.
	IDXGISwapChain*          pDxSwapChain;
	/// Sync interval to specify how interval for vsync
	uint32_t                 mDxSyncInterval : 3;
	uint32_t                 mFlags : 10;
	uint32_t                 mImageCount : 3;
	uint32_t                 mEnableVsync : 1;
	uint32_t                 mPadA;
	uint64_t                 mPadB[5];
#endif
#if defined(VULKAN)
	/// Present queue if one exists (queuePresent will use this queue if the hardware has a dedicated present queue)
	VkQueue                  pPresentQueue;
	VkSwapchainKHR           pSwapChain;
	VkSurfaceKHR             pVkSurface;
	SwapChainDesc*           pDesc;
	uint32_t                 mPresentQueueFamilyIndex : 5;
	uint32_t                 mImageCount : 3;
	uint32_t                 mEnableVsync : 1;
	uint32_t                 mPadA;
#endif
#if defined(METAL)
#if defined(TARGET_IOS)
	UIView*                  pForgeView;
#else
	NSView*                  pForgeView;
#endif
	id<CAMetalDrawable>      mMTKDrawable;
	id<MTLCommandBuffer>     presentCommandBuffer;
	uint32_t                 mImageCount : 3;
	uint32_t                 mEnableVsync : 1;
	uint32_t                 mPadA;
	uint64_t                 mPadB[4];
#endif
#if defined(ORBIS)
	OrbisSwapChain           mStruct;
	uint32_t                 mImageCount : 3;
	uint32_t                 mEnableVsync : 1;
#endif
} SwapChain;

typedef enum ShaderTarget
{
	// We only need SM 5.0 for supporting D3D11 fallback
#if defined(DIRECT3D11)
	shader_target_5_0,
#else
	// 5.1 is supported on all DX12 hardware
	shader_target_5_1,
	shader_target_6_0,
	shader_target_6_1,
	shader_target_6_2,
	shader_target_6_3, //required for Raytracing
#endif
} ShaderTarget;

typedef enum GpuMode
{
	GPU_MODE_SINGLE = 0,
	GPU_MODE_LINKED,
	// #TODO GPU_MODE_UNLINKED,
} GpuMode;

typedef struct RendererDesc
{
	LogFn                        pLogFn;
	RendererApi                  mApi;
	ShaderTarget                 mShaderTarget;
	GpuMode                      mGpuMode;
#if defined(VULKAN)
	const char**                 ppInstanceLayers;
	uint32_t                     mInstanceLayerCount;
	const char**                 ppInstanceExtensions;
	uint32_t                     mInstanceExtensionCount;
	const char**                 ppDeviceExtensions;
	uint32_t                     mDeviceExtensionCount;
#endif
#if defined(DIRECT3D12)
	D3D_FEATURE_LEVEL            mDxFeatureLevel;
#endif
	/// This results in new validation not possible during API calls on the CPU, by creating patched shaders that have validation added directly to the shader.
	/// However, it can slow things down a lot, especially for applications with numerous PSOs. Time to see the first render frame may take several minutes
	bool                         mEnableGPUBasedValidation;
} RendererDesc;

typedef struct GPUVendorPreset
{
	char           mVendorId[MAX_GPU_VENDOR_STRING_LENGTH];
	char           mModelId[MAX_GPU_VENDOR_STRING_LENGTH];
	char           mRevisionId[MAX_GPU_VENDOR_STRING_LENGTH];    // OPtional as not all gpu's have that. Default is : 0x00
	GPUPresetLevel mPresetLevel;
	char           mGpuName[MAX_GPU_VENDOR_STRING_LENGTH];    //If GPU Name is missing then value will be empty string
} GPUVendorPreset;

typedef struct GPUCapBits
{
	bool canShaderReadFrom[TinyImageFormat_Count];
	bool canShaderWriteTo[TinyImageFormat_Count];
	bool canRenderTargetWriteTo[TinyImageFormat_Count];
} GPUCapBits;

typedef enum DefaultResourceAlignment
{
	RESOURCE_BUFFER_ALIGNMENT = 4U,
} DefaultResourceAlignment;

typedef struct GPUSettings
{
	uint32_t        mUniformBufferAlignment;
	uint32_t        mUploadBufferTextureAlignment;
	uint32_t        mUploadBufferTextureRowAlignment;
	uint32_t        mMaxVertexInputBindings;
	uint32_t        mMaxRootSignatureDWORDS;
	uint32_t        mWaveLaneCount;
	GPUVendorPreset mGpuVendorPreset;
	bool            mMultiDrawIndirect;
	bool            mROVsSupported;
#ifdef METAL
	uint32_t        mArgumentBufferMaxTextures;
#endif
} GPUSettings;

typedef struct DEFINE_ALIGNED(Renderer, 64)
{
#if defined(DIRECT3D12)
	// API specific descriptor heap and memory allocator
	struct DescriptorHeap**         pCPUDescriptorHeaps;
	struct DescriptorHeap**         pCbvSrvUavHeaps;
	struct DescriptorHeap**         pSamplerHeaps;
	class  D3D12MA::Allocator*      pResourceAllocator;
#if defined(_DURANGO)
	IDXGIFactory2*                  pDXGIFactory;
	IDXGIAdapter*                   pDxActiveGPU;
	ID3D12Device*                   pDxDevice;
	EsramManager*                   pESRAMManager;
#elif defined(DIRECT3D12)
	IDXGIFactory6*                  pDXGIFactory;
	IDXGIAdapter4*                  pDxActiveGPU;
	ID3D12Device*                   pDxDevice;
	uint64_t                        mPadA;
#endif
	ID3D12Debug*                    pDXDebug;
	uint64_t                        mPadB;
#endif
#if defined(DIRECT3D11)
	IDXGIFactory1*                  pDXGIFactory;
	IDXGIAdapter1*                  pDxActiveGPU;
	ID3D11Device*                   pDxDevice;
	ID3D11DeviceContext*            pDxContext;
	ID3D11BlendState*               pDefaultBlendState;
	ID3D11DepthStencilState*        pDefaultDepthState;
	ID3D11RasterizerState*          pDefaultRasterizerState;
	uint32_t                        mPartialUpdateConstantBufferSupported : 1;
	uint32_t                        mPadA;
	uint64_t                        mPadB;
	uint64_t                        mPadC;
#endif
#if defined(VULKAN)
	VkInstance                      pVkInstance;
	VkPhysicalDevice                pVkActiveGPU;
	VkPhysicalDeviceProperties2*    pVkActiveGPUProperties;
	VkDevice                        pVkDevice;
#ifdef USE_DEBUG_UTILS_EXTENSION
	VkDebugUtilsMessengerEXT        pVkDebugUtilsMessenger;
#else
	VkDebugReportCallbackEXT        pVkDebugReport;
#endif
	uint32_t**                      pUsedQueueCount;
	DescriptorPool*          pDescriptorPool;
	VmaAllocator_T*          pVmaAllocator;
	uint32_t                        mRaytracingExtension : 1;
	uint32_t                        mPadA;
	uint64_t                        mPadB;
#endif
#if defined(METAL)
	id<MTLDevice>                   pDevice;
	struct ResourceAllocator*       pResourceAllocator;
	uint64_t                        mPadA[8];
#endif
#if defined(ORBIS)
	uint64_t                        mPadA;
	uint64_t                        mPadB;
#endif
	struct NullDescriptors*         pNullDescriptors;
	char*                           pName;
	GPUSettings*                    pActiveGpuSettings;
	ShaderMacro*                    pBuiltinShaderDefines;
	GPUCapBits*                     pCapBits;
	uint32_t                        mLinkedNodeCount : 4;
	uint32_t                        mGpuMode : 3;
	uint32_t                        mShaderTarget : 4;
	uint32_t                        mApi : 5;
	uint32_t                        mEnableGpuBasedValidation : 1;
	uint32_t                        mBuiltinShaderDefinesCount;
} Renderer;
#if defined(ORBIS)
COMPILE_ASSERT(sizeof(Renderer) == 8 * sizeof(uint64_t));
#else
// 2 cache lines
COMPILE_ASSERT(sizeof(Renderer) == 16 * sizeof(uint64_t));
#endif

// Indirect command sturcture define
typedef struct IndirectArgumentDescriptor
{
	IndirectArgumentType mType;
	const char*          pName;
	uint32_t             mIndex;
	uint32_t             mCount;
	uint32_t             mDivisor;

} IndirectArgumentDescriptor;

typedef struct CommandSignatureDesc
{
	CmdPool*                    pCmdPool;
	RootSignature*              pRootSignature;
	uint32_t                    mIndirectArgCount;
	IndirectArgumentDescriptor* pArgDescs;
} CommandSignatureDesc;

typedef struct CommandSignature
{
#if defined(DIRECT3D12)
	ID3D12CommandSignature* pDxHandle;
#endif
#if defined(VULKAN)
	IndirectArgumentType    mDrawType;
	uint32_t                mStride;
#endif
#if defined(METAL)
	IndirectArgumentType    mDrawType;
	uint32_t                mPadA;
#endif
#if defined(ORBIS)
	IndirectArgumentType    mDrawType;
	uint32_t                mStride;
#endif
} CommandSignature;

typedef struct DescriptorSetDesc
{
	RootSignature*             pRootSignature;
	DescriptorUpdateFrequency  mUpdateFrequency;
	uint32_t                   mMaxSets;
	uint32_t                   mNodeIndex;
	//    const wchar_t*             pDebugName;
} DescriptorSetDesc;

typedef struct QueueSubmitDesc
{
	uint32_t    mCmdCount;
	Cmd**       ppCmds;
	Fence*      pSignalFence;
	uint32_t    mWaitSemaphoreCount;
	Semaphore** ppWaitSemaphores;
	uint32_t    mSignalSemaphoreCount;
	Semaphore** ppSignalSemaphores;
	bool        mSubmitDone;
} QueueSubmitDesc;

typedef struct QueuePresentDesc
{
	SwapChain*  pSwapChain;
	uint32_t    mWaitSemaphoreCount;
	Semaphore** ppWaitSemaphores;
	uint8_t     mIndex;
	bool        mSubmitDone;
} QueuePresentDesc;

// clang-format off
// API functions
// allocates memory and initializes the renderer -> returns pRenderer
//
void FORGE_CALLCONV initRenderer(const char* app_name, const RendererDesc* p_settings, Renderer** pRenderer);
void FORGE_CALLCONV removeRenderer(Renderer* pRenderer);

void FORGE_CALLCONV addFence(Renderer* pRenderer, Fence** p_fence);
void FORGE_CALLCONV removeFence(Renderer* pRenderer, Fence* p_fence);

void FORGE_CALLCONV addSemaphore(Renderer* pRenderer, Semaphore** p_semaphore);
void FORGE_CALLCONV removeSemaphore(Renderer* pRenderer, Semaphore* p_semaphore);

void FORGE_CALLCONV addQueue(Renderer* pRenderer, QueueDesc* pQDesc, Queue** pQueue);
void FORGE_CALLCONV removeQueue(Renderer* pRenderer, Queue* pQueue);

void FORGE_CALLCONV addSwapChain(Renderer* pRenderer, const SwapChainDesc* p_desc, SwapChain** p_swap_chain);
void FORGE_CALLCONV removeSwapChain(Renderer* pRenderer, SwapChain* p_swap_chain);

// command pool functions
void FORGE_CALLCONV addCmdPool(Renderer* pRenderer, const CmdPoolDesc* p_desc, CmdPool** p_cmd_pool);
void FORGE_CALLCONV removeCmdPool(Renderer* pRenderer, CmdPool* p_CmdPool);
void FORGE_CALLCONV addCmd(Renderer* pRenderer, const CmdDesc* p_desc, Cmd** p_cmd);
void FORGE_CALLCONV removeCmd(Renderer* pRenderer, Cmd* pCmd);
void FORGE_CALLCONV addCmd_n(Renderer* pRenderer, const CmdDesc* p_desc, uint32_t cmd_count, Cmd*** p_cmds);
void FORGE_CALLCONV removeCmd_n(Renderer* pRenderer, uint32_t cmd_count, Cmd** p_cmds);

//
// All buffer, texture loading handled by resource system -> IResourceLoader.*
//

void FORGE_CALLCONV addRenderTarget(Renderer* pRenderer, const RenderTargetDesc* p_desc, RenderTarget** p_render_target);
void FORGE_CALLCONV removeRenderTarget(Renderer* pRenderer, RenderTarget* p_render_target);
void FORGE_CALLCONV addSampler(Renderer* pRenderer, const SamplerDesc* pDesc, Sampler** p_sampler);
void FORGE_CALLCONV removeSampler(Renderer* pRenderer, Sampler* p_sampler);

// shader functions
#if defined(TARGET_IOS)
void FORGE_CALLCONV addShader(Renderer* pRenderer, const ShaderDesc* p_desc, Shader** p_shader_program);
#endif
void FORGE_CALLCONV addShaderBinary(Renderer* pRenderer, const BinaryShaderDesc* p_desc, Shader** p_shader_program);
void FORGE_CALLCONV removeShader(Renderer* pRenderer, Shader* p_shader_program);

void FORGE_CALLCONV addRootSignature(Renderer* pRenderer, const RootSignatureDesc* pDesc, RootSignature** pRootSignature);
void FORGE_CALLCONV removeRootSignature(Renderer* pRenderer, RootSignature* pRootSignature);

// pipeline functions
void FORGE_CALLCONV addPipeline(Renderer* pRenderer, const PipelineDesc* p_pipeline_settings, Pipeline** p_pipeline);
void FORGE_CALLCONV removePipeline(Renderer* pRenderer, Pipeline* p_pipeline);

// Descriptor Set functions
void FORGE_CALLCONV addDescriptorSet(Renderer* pRenderer, const DescriptorSetDesc* pDesc, DescriptorSet** pDescriptorSet);
void FORGE_CALLCONV removeDescriptorSet(Renderer* pRenderer, DescriptorSet* pDescriptorSet);
void FORGE_CALLCONV updateDescriptorSet(Renderer* pRenderer, uint32_t index, DescriptorSet* pDescriptorSet, uint32_t count, const DescriptorData* pParams);

// command buffer functions
void FORGE_CALLCONV beginCmd(Cmd* p_cmd);
void FORGE_CALLCONV endCmd(Cmd* p_cmd);
void FORGE_CALLCONV cmdBindRenderTargets(Cmd* p_cmd, uint32_t render_target_count, RenderTarget** p_render_targets, RenderTarget* p_depth_stencil, const LoadActionsDesc* loadActions, uint32_t* pColorArraySlices, uint32_t* pColorMipSlices, uint32_t depthArraySlice, uint32_t depthMipSlice);
void FORGE_CALLCONV cmdSetViewport(Cmd* p_cmd, float x, float y, float width, float height, float min_depth, float max_depth);
void FORGE_CALLCONV cmdSetScissor(Cmd* p_cmd, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void FORGE_CALLCONV cmdBindPipeline(Cmd* p_cmd, Pipeline* p_pipeline);
void FORGE_CALLCONV cmdBindDescriptorSet(Cmd* pCmd, uint32_t index, DescriptorSet* pDescriptorSet);
void FORGE_CALLCONV cmdBindPushConstants(Cmd* pCmd, RootSignature* pRootSignature, const char* pName, const void* pConstants);
void FORGE_CALLCONV cmdBindPushConstantsByIndex(Cmd* pCmd, RootSignature* pRootSignature, uint32_t paramIndex, const void* pConstants);
void FORGE_CALLCONV cmdBindIndexBuffer(Cmd* p_cmd, Buffer* p_buffer, uint32_t indexType, uint64_t offset);
void FORGE_CALLCONV cmdBindVertexBuffer(Cmd* p_cmd, uint32_t buffer_count, Buffer** pp_buffers, const uint32_t* pStrides, const uint64_t* pOffsets);
void FORGE_CALLCONV cmdDraw(Cmd* p_cmd, uint32_t vertex_count, uint32_t first_vertex);
void FORGE_CALLCONV cmdDrawInstanced(Cmd* pCmd, uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount, uint32_t firstInstance);
void FORGE_CALLCONV cmdDrawIndexed(Cmd* p_cmd, uint32_t index_count, uint32_t first_index, uint32_t first_vertex);
void FORGE_CALLCONV cmdDrawIndexedInstanced(Cmd* pCmd, uint32_t indexCount, uint32_t firstIndex, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
void FORGE_CALLCONV cmdDispatch(Cmd* p_cmd, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

// Transition Commands
void FORGE_CALLCONV cmdResourceBarrier(Cmd* p_cmd, uint32_t buffer_barrier_count, BufferBarrier* p_buffer_barriers, uint32_t texture_barrier_count, TextureBarrier* p_texture_barriers, uint32_t rt_barrier_count, RenderTargetBarrier* p_rt_barriers);

// Virtual Textures

void FORGE_CALLCONV cmdUpdateVirtualTexture(Cmd* pCmd, Texture* pTexture);

//
// All buffer, texture update handled by resource system -> IResourceLoader.*
//

// queue/fence/swapchain functions
void FORGE_CALLCONV acquireNextImage(Renderer* pRenderer, SwapChain* p_swap_chain, Semaphore* p_signal_semaphore, Fence* p_fence, uint32_t* p_image_index);
void FORGE_CALLCONV queueSubmit(Queue* p_queue, const QueueSubmitDesc* p_desc);
void FORGE_CALLCONV queuePresent(Queue* p_queue, const QueuePresentDesc* p_desc);
void FORGE_CALLCONV waitQueueIdle(Queue* p_queue);
void FORGE_CALLCONV getFenceStatus(Renderer* pRenderer, Fence* p_fence, FenceStatus* p_fence_status);
void FORGE_CALLCONV waitForFences(Renderer* pRenderer, uint32_t fenceCount, Fence** ppFences);
void FORGE_CALLCONV toggleVSync(Renderer* pRenderer, SwapChain** ppSwapchain);

//Returns the recommended format for the swapchain.
//If true is passed for the hintHDR parameter, it will return an HDR format IF the platform supports it
//If false is passed or the platform does not support HDR a non HDR format is returned.
TinyImageFormat FORGE_CALLCONV getRecommendedSwapchainFormat(bool hintHDR);

//indirect Draw functions
void FORGE_CALLCONV addIndirectCommandSignature(Renderer* pRenderer, const CommandSignatureDesc* p_desc, CommandSignature** ppCommandSignature);
void FORGE_CALLCONV removeIndirectCommandSignature(Renderer* pRenderer, CommandSignature* pCommandSignature);
void FORGE_CALLCONV cmdExecuteIndirect(Cmd* pCmd, CommandSignature* pCommandSignature, uint32_t maxCommandCount, Buffer* pIndirectBuffer, uint64_t bufferOffset, Buffer* pCounterBuffer, uint64_t counterBufferOffset);
/************************************************************************/
// GPU Query Interface
/************************************************************************/
void FORGE_CALLCONV getTimestampFrequency(Queue* pQueue, double* pFrequency);
void FORGE_CALLCONV addQueryPool(Renderer* pRenderer, const QueryPoolDesc* pDesc, QueryPool** ppQueryPool);
void FORGE_CALLCONV removeQueryPool(Renderer* pRenderer, QueryPool* pQueryPool);
void FORGE_CALLCONV cmdResetQueryPool(Cmd* pCmd, QueryPool* pQueryPool, uint32_t startQuery, uint32_t queryCount);
void FORGE_CALLCONV cmdBeginQuery(Cmd* pCmd, QueryPool* pQueryPool, QueryDesc* pQuery);
void FORGE_CALLCONV cmdEndQuery(Cmd* pCmd, QueryPool* pQueryPool, QueryDesc* pQuery);
void FORGE_CALLCONV cmdResolveQuery(Cmd* pCmd, QueryPool* pQueryPool, Buffer* pReadbackBuffer, uint32_t startQuery, uint32_t queryCount);
/************************************************************************/
// Stats Info Interface
/************************************************************************/
void FORGE_CALLCONV calculateMemoryStats(Renderer* pRenderer, char** stats);
void FORGE_CALLCONV freeMemoryStats(Renderer* pRenderer, char* stats);
/************************************************************************/
// Debug Marker Interface
/************************************************************************/
void FORGE_CALLCONV cmdBeginDebugMarker(Cmd* pCmd, float r, float g, float b, const char* pName);
void FORGE_CALLCONV cmdEndDebugMarker(Cmd* pCmd);
void FORGE_CALLCONV cmdAddDebugMarker(Cmd* pCmd, float r, float g, float b, const char* pName);
/************************************************************************/
// Resource Debug Naming Interface
/************************************************************************/
void FORGE_CALLCONV setBufferName(Renderer* pRenderer, Buffer* pBuffer, const char* pName);
void FORGE_CALLCONV setTextureName(Renderer* pRenderer, Texture* pTexture, const char* pName);
/************************************************************************/

SE_NAMESPACE_END
//=============================================================================