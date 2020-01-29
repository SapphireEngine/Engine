#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

/**
* - Direct3D 12: "D3D12_DRAW_ARGUMENTS"
* - Direct3D 11: No structure documentation found, only indications that same arguments and order as "ID3D11DeviceContext::DrawInstanced()"
* - Vulkan: "VkDrawIndirectCommand"
* - OpenGL:"DrawArraysIndirectCommand"
*/
struct DrawArguments final
{
	uint32_t vertexCountPerInstance;
	uint32_t instanceCount;
	uint32_t startVertexLocation;
	uint32_t startInstanceLocation;
	inline DrawArguments(uint32_t _vertexCountPerInstance, uint32_t _instanceCount = 1, uint32_t _startVertexLocation = 0, uint32_t _startInstanceLocation = 0) :
		vertexCountPerInstance(_vertexCountPerInstance),
		instanceCount(_instanceCount),
		startVertexLocation(_startVertexLocation),
		startInstanceLocation(_startInstanceLocation)
	{
	}
};

/**
* - Direct3D 12: "D3D12_DRAW_INDEXED_ARGUMENTS"
* - Direct3D 11: No structure documentation found, only indications that same arguments and order as "ID3D11DeviceContext::DrawIndexedInstanced()"
* - Vulkan: "VkDrawIndexedIndirectCommand"
* - OpenGL:"DrawElementsIndirectCommand"
*/
struct DrawIndexedArguments final
{
	uint32_t indexCountPerInstance;
	uint32_t instanceCount;
	uint32_t startIndexLocation;
	int32_t  baseVertexLocation;
	uint32_t startInstanceLocation;
	inline DrawIndexedArguments(uint32_t _indexCountPerInstance, uint32_t _instanceCount = 1, uint32_t _startIndexLocation = 0, int32_t _baseVertexLocation = 0, uint32_t _startInstanceLocation = 0) :
		indexCountPerInstance(_indexCountPerInstance),
		instanceCount(_instanceCount),
		startIndexLocation(_startIndexLocation),
		baseVertexLocation(_baseVertexLocation),
		startInstanceLocation(_startInstanceLocation)
	{
	}
};


SE_NAMESPACE_END
//=============================================================================