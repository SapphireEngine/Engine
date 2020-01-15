#pragma once

#include "Core/Memory/Memory.h"

//=============================================================================
SE_NAMESPACE_BEGIN

namespace detail
{
	[[nodiscard]] VKAPI_ATTR void* VKAPI_CALL vkAllocationFunction(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope)
	{
		return reinterpret_cast<IAllocator*>(pUserData)->Reallocate(nullptr, 0, size, alignment);
	}

	[[nodiscard]] VKAPI_ATTR void* VKAPI_CALL vkReallocationFunction(void *pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope)
	{
		return reinterpret_cast<IAllocator*>(pUserData)->Reallocate(pOriginal, 0, size, alignment);
	}

	VKAPI_ATTR void VKAPI_CALL vkFreeFunction(void *pUserData, void *pMemory)
	{
		reinterpret_cast<IAllocator*>(pUserData)->Reallocate(pMemory, 0, 0, 1);
	}
}

SE_NAMESPACE_END
//=============================================================================