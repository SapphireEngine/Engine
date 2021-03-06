#pragma once

#include "Platform/FileSystem/FileSystemInternal.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class MemoryStream : public FileStream
{
	uint8_t* pBuffer;
	size_t   mBufferSize;
	size_t   mCursor;
	bool     mReadOnly;

public:
	inline MemoryStream(uint8_t* buffer, size_t bufferSize, bool readOnly) :
		FileStream(FileStreamType_MemoryStream, NULL),
		pBuffer(buffer),
		mBufferSize(bufferSize),
		mCursor(0),
		mReadOnly(readOnly)
	{
	}

	inline size_t AvailableCapacity(size_t requestedCapacity) const
	{
		return min((ssize_t)requestedCapacity, max((ssize_t)mBufferSize - (ssize_t)mCursor, (ssize_t)0));
	}

	size_t  Read(void* outputBuffer, size_t bufferSizeInBytes) override;
	size_t  Scan(const char* format, va_list args, int* bytesRead) override;
	size_t  Write(const void* sourceBuffer, size_t byteCount) override;
	size_t  Print(const char* format, va_list args) override;
	bool    Seek(SeekBaseOffset baseOffset, ssize_t seekOffset) override;
	ssize_t GetSeekPosition() const override;
	ssize_t GetFileSize() const override;
	void*   GetUnderlyingBuffer() const override;
	void    Flush() override;
	bool    IsAtEnd() const override;
	bool    Close() override;
};

SE_NAMESPACE_END
//=============================================================================