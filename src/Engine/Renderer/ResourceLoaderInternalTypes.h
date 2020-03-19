#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

#ifdef __cplusplus
extern "C" {
#endif

	// The ResourceLoader UpdateDesc and LoadDesc types store internal state at the end of the struct.
	// The types for this internal state need to be defined, but we don't want them cluttering up the ResourceLoader interface,
	// so instead we define them here.

	typedef struct MappedMemoryRange
	{
		uint8_t* p_data;
		Buffer*  pBuffer;
		uint64_t mOffset;
		uint64_t mSize;
	} MappedMemoryRange;

	typedef struct UMAAllocation
	{
		void* p_data;
		void* pAllocationInfo;
	} UMAAllocation;

	typedef struct BufferUpdateInternalData
	{
		MappedMemoryRange mMappedRange;
		bool mBufferNeedsUnmap;
	} BufferUpdateInternalData;

#ifdef __cplusplus
} // extern "C"
#endif

SE_NAMESPACE_END
//=============================================================================