#pragma once

#include "Core/Memory/IAllocator.h"

//=============================================================================
SE_NAMESPACE_BEGIN

namespace detail
{
	[[nodiscard]] void* Reallocate(IAllocator&, void *oldPointer, size_t, size_t newNumberOfBytes, size_t alignment)
	{
		// Sanity check
		SE_ASSERT(0 != alignment && !(alignment & (alignment - 1)), "The alignment must be a power of two")

			// Do the work
			if ( nullptr != oldPointer && 0 != newNumberOfBytes )
			{
				// Reallocate
#ifdef _MSC_VER
				return _aligned_realloc(oldPointer, newNumberOfBytes, alignment);
#else
	// TODO(co) Need aligned version, see e.g. https://github.com/philiptaylor/vulkan-sxs/blob/9cb21b3/common/AllocationCallbacks.cpp#L87 or XSIMD "xsimd_aligned_allocator"
				return realloc(oldPointer, newNumberOfBytes);
#endif
			}
			else
			{
				// Malloc / free
#ifdef _MSC_VER
	// Null pointer is valid in here, does nothing in this case
				::_aligned_free(oldPointer);

				// Allocate
				return (0 != newNumberOfBytes) ? ::_aligned_malloc(newNumberOfBytes, alignment) : nullptr;
#elif defined(__ANDROID__)
	// Null pointer is valid in here, does nothing in this case
				::free(oldPointer);

				// Allocate
				if ( 0 != newNumberOfBytes )
				{
					void* memptr = nullptr;
					return (posix_memalign(&memptr, alignment, newNumberOfBytes) == 0) ? memptr : nullptr;
				}
				else
				{
					return nullptr;
				}
#else
	// Null pointer is valid in here, does nothing in this case
				::free(oldPointer);

				// Allocate
				return (0 != newNumberOfBytes) ? ::aligned_alloc(alignment, newNumberOfBytes) : nullptr;
#endif
			}
	}
} // detail

class DefaultAllocator final : public IAllocator
{
public:
	DefaultAllocator() : IAllocator(&detail::Reallocate){}
private:
	explicit DefaultAllocator(const DefaultAllocator&) = delete;
	DefaultAllocator& operator=(const DefaultAllocator&) = delete;
};

SE_NAMESPACE_END
//=============================================================================