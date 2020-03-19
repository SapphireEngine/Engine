#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/IResourceLoader.h"

//=============================================================================
SE_NAMESPACE_BEGIN

/************************************************************************/
/* RING BUFFER MANAGEMENT											  */
/************************************************************************/
typedef struct GPURingBuffer
{
	Renderer* pRenderer;
	Buffer*   pBuffer;

	uint32_t mBufferAlignment;
	uint64_t mMaxBufferSize;
	uint64_t mCurrentBufferOffset;
} GPURingBuffer;


typedef struct GPURingBufferOffset
{
	Buffer*  pBuffer;
	uint64_t mOffset;
} GPURingBufferOffset;

static inline void addGPURingBuffer(Renderer* pRenderer, const BufferDesc* pBufferDesc, GPURingBuffer** ppRingBuffer)
{
	GPURingBuffer* pRingBuffer = (GPURingBuffer*)conf_calloc(1, sizeof(GPURingBuffer));
	pRingBuffer->pRenderer = pRenderer;
	pRingBuffer->mMaxBufferSize = pBufferDesc->mSize;
	pRingBuffer->mBufferAlignment = sizeof(float[4]);
	BufferLoadDesc loadDesc = {};
	loadDesc.mDesc = *pBufferDesc;
	loadDesc.ppBuffer = &pRingBuffer->pBuffer;
	addResource(&loadDesc, NULL, LOAD_PRIORITY_NORMAL);

	*ppRingBuffer = pRingBuffer;
}

static inline void addUniformGPURingBuffer(Renderer* pRenderer, uint32_t requiredUniformBufferSize, GPURingBuffer** ppRingBuffer, bool const ownMemory = false, ResourceMemoryUsage memoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU)
{
	GPURingBuffer* pRingBuffer = (GPURingBuffer*)conf_calloc(1, sizeof(GPURingBuffer));
	pRingBuffer->pRenderer = pRenderer;

	const uint32_t uniformBufferAlignment = (uint32_t)pRenderer->pActiveGpuSettings->mUniformBufferAlignment;
	const uint32_t maxUniformBufferSize = requiredUniformBufferSize;
	pRingBuffer->mBufferAlignment = uniformBufferAlignment;
	pRingBuffer->mMaxBufferSize = maxUniformBufferSize;

	BufferDesc ubDesc = {};
#if defined(DIRECT3D11)
	ubDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_ONLY;
#else
	ubDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubDesc.mMemoryUsage = memoryUsage;
#endif
	ubDesc.mFlags = (ubDesc.mMemoryUsage != RESOURCE_MEMORY_USAGE_GPU_ONLY ? BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT : BUFFER_CREATION_FLAG_NONE) |
		BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION;
	if ( ownMemory )
		ubDesc.mFlags |= BUFFER_CREATION_FLAG_OWN_MEMORY_BIT;
	ubDesc.mSize = maxUniformBufferSize;
	BufferLoadDesc loadDesc = {};
	loadDesc.mDesc = ubDesc;
	loadDesc.ppBuffer = &pRingBuffer->pBuffer;
	addResource(&loadDesc, NULL, LOAD_PRIORITY_NORMAL);

	*ppRingBuffer = pRingBuffer;
}

static inline void removeGPURingBuffer(GPURingBuffer* pRingBuffer)
{
	removeResource(pRingBuffer->pBuffer);
	conf_free(pRingBuffer);
}

static inline void resetGPURingBuffer(GPURingBuffer* pRingBuffer)
{
	pRingBuffer->mCurrentBufferOffset = 0;
}

static inline GPURingBufferOffset getGPURingBufferOffset(GPURingBuffer* pRingBuffer, uint32_t memoryRequirement, uint32_t alignment = 0)
{
	uint32_t alignedSize = round_up(memoryRequirement, alignment ? alignment : pRingBuffer->mBufferAlignment);

	if ( alignedSize > pRingBuffer->mMaxBufferSize )
	{
		SE_ASSERT(false && "Ring Buffer too small for memory requirement");
		return { NULL, 0 };
	}

	if ( pRingBuffer->mCurrentBufferOffset + alignedSize >= pRingBuffer->mMaxBufferSize )
	{
		pRingBuffer->mCurrentBufferOffset = 0;
	}

	GPURingBufferOffset ret = { pRingBuffer->pBuffer, pRingBuffer->mCurrentBufferOffset };
	pRingBuffer->mCurrentBufferOffset += alignedSize;

	return ret;
}

SE_NAMESPACE_END
//=============================================================================