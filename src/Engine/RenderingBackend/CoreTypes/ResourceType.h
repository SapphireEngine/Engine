#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

enum class ResourceType
{
	ROOT_SIGNATURE = 0,		// Root signature
	RESOURCE_GROUP = 1,		// Resource group
	GRAPHICS_PROGRAM = 2,	// Graphics program, "IShader"-related
	VERTEX_ARRAY = 3,		// Vertex array object (VAO, input-assembler (IA) stage), "IBuffer"-related
	RENDER_PASS = 4,		// Render pass
	QUERY_POOL = 5,			// Asynchronous query pool
	// IRenderTarget
	SWAP_CHAIN = 6,			// Swap chain
	FRAMEBUFFER = 7,		// Framebuffer object (FBO)
	// IBuffer
	VERTEX_BUFFER = 8,		// Vertex buffer object (VBO, input-assembler (IA) stage)
	INDEX_BUFFER = 9,		// Index buffer object (IBO, input-assembler (IA) stage)
	TEXTURE_BUFFER = 10,	// Texture buffer object (TBO)
	STRUCTURED_BUFFER = 11,	// Structured buffer object (SBO)
	INDIRECT_BUFFER = 12,	// Indirect buffer object
	UNIFORM_BUFFER = 13,	// Uniform buffer object (UBO, "constant buffer" in Direct3D terminology)
	// ITexture
	TEXTURE_1D = 14,		// Texture 1D
	TEXTURE_1D_ARRAY = 15,	// Texture 1D array
	TEXTURE_2D = 16,		// Texture 2D
	TEXTURE_2D_ARRAY = 17,	// Texture 2D array
	TEXTURE_3D = 18,		// Texture 3D
	TEXTURE_CUBE = 19,		// Texture cube
	TEXTURE_CUBE_ARRAY = 20,// Texture cube array
	// IPipelineState
	GRAPHICS_PIPELINE_STATE = 21,// Graphics pipeline state (PSO)
	COMPUTE_PIPELINE_STATE = 22,// Compute pipeline state (PSO)
	SAMPLER_STATE = 23,		// Sampler state
	// IShader
	VERTEX_SHADER = 24,		// Vertex shader (VS)
	TESSELLATION_CONTROL_SHADER = 25,// Tessellation control shader (TCS, "hull shader" in Direct3D terminology)
	TESSELLATION_EVALUATION_SHADER = 26,// Tessellation evaluation shader (TES, "domain shader" in Direct3D terminology)
	GEOMETRY_SHADER = 27,	// Geometry shader (GS)
	FRAGMENT_SHADER = 28,	// Fragment shader (FS, "pixel shader" in Direct3D terminology)
	COMPUTE_SHADER = 29		// Compute shader (CS)
};

SE_NAMESPACE_END
//=============================================================================