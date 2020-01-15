#pragma once

#include "Core/Debug/Assert.h"

//=============================================================================
SE_NAMESPACE_BEGIN

// http://nfrechette.github.io/2015/05/11/memory_allocator_interface/

class IAllocator
{
public:
	virtual ~IAllocator() = default;

	/**
	*  @brief
	*    Reallocate
	*
	*  @param[in] oldPointer
	*    Old pointer, can be a null pointer
	*  @param[in] oldNumberOfBytes
	*    Old number of bytes, must be zero of the old pointer is a null pointer, else can be zero if this information isn't available
	*  @param[in] newNumberOfBytes
	*    New number of bytes
	*  @param[in] alignment
	*    Alignment
	*
	*  @return
	*    New pointer, can be a null pointer
	*/
	inline void* Reallocate(void *oldPointer, size_t oldNumberOfBytes, size_t newNumberOfBytes, size_t alignment)
	{
		SE_ASSERT(nullptr != m_reallocateFuntion, "Invalid reallocate function");
		SE_ASSERT(nullptr != oldPointer || 0 == oldNumberOfBytes, "Invalid old pointer");
		return (*m_reallocateFuntion)(*this, oldPointer, oldNumberOfBytes, newNumberOfBytes, alignment);
	}

protected:
	typedef void* (*ReallocateFuntion)(IAllocator&, void*, size_t, size_t, size_t);

	explicit IAllocator(ReallocateFuntion reallocateFuntion) : m_reallocateFuntion(reallocateFuntion)
	{
		SE_ASSERT(nullptr != m_reallocateFuntion, "Invalid reallocate funtion")
	}

	ReallocateFuntion m_reallocateFuntion;
};

SE_NAMESPACE_END
//=============================================================================