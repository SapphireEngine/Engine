#include "stdafx.h"
#include "MemoryStream.h"
#include "Core/Debug/Log.h"

//=============================================================================
SE_NAMESPACE_BEGIN

FileStream* fsOpenReadOnlyMemory(const void* buffer, size_t bufferSize)
{
	return conf_new(MemoryStream, (uint8_t*)buffer, bufferSize, true);
}

FileStream* fsOpenReadWriteMemory(void* buffer, size_t bufferSize)
{
	return conf_new(MemoryStream, (uint8_t*)buffer, bufferSize, false);
}

size_t MemoryStream::Read(void* outputBuffer, size_t bufferSizeInBytes)
{
	size_t bytesToRead = AvailableCapacity(bufferSizeInBytes);
	memcpy(outputBuffer, pBuffer + mCursor, bytesToRead);
	mCursor += bytesToRead;
	return bytesToRead;
}

size_t MemoryStream::Scan(const char* format, va_list args, int* bytesRead)
{
	size_t itemsScanned = vsscanf((const char*)pBuffer + mCursor, format, args);
	mCursor += *bytesRead;
	return itemsScanned;
}

size_t MemoryStream::Write(const void* sourceBuffer, size_t byteCount)
{
	if ( mReadOnly )
	{
		LOGF(LogLevel::eWARNING, "Attempting to write to read-only buffer at %p", pBuffer);
		return 0;
	}
	size_t bytesToWrite = AvailableCapacity(byteCount);
	memcpy(pBuffer + mCursor, sourceBuffer, bytesToWrite);
	return bytesToWrite;
}

size_t MemoryStream::Print(const char* format, va_list args)
{
	size_t bytesToWrite = max((ssize_t)mBufferSize - (ssize_t)mCursor, (ssize_t)0);
	size_t bytesNeeded = vsnprintf((char*)pBuffer + mCursor, bytesToWrite, format, args);
	size_t bytesWritten = min(bytesNeeded, bytesToWrite);
	mCursor += bytesWritten;
	return bytesWritten;
}

bool MemoryStream::Seek(SeekBaseOffset baseOffset, ssize_t seekOffset)
{
	switch ( baseOffset )
	{
	case SBO_START_OF_FILE:
		{
			if ( seekOffset < 0 || (size_t)seekOffset >= mBufferSize )
			{
				return false;
			}
			mCursor = seekOffset;
		}
		break;
	case SBO_CURRENT_POSITION:
		{
			ssize_t newPosition = (ssize_t)mCursor + seekOffset;
			if ( newPosition < 0 || (size_t)newPosition >= mBufferSize )
			{
				return false;
			}
			mCursor = (size_t)newPosition;
		}
		break;

	case SBO_END_OF_FILE:
		{
			ssize_t newPosition = (ssize_t)mBufferSize + seekOffset;
			if ( newPosition < 0 || (size_t)newPosition >= mBufferSize )
			{
				return false;
			}
			mCursor = (size_t)newPosition;
		}
		break;
	}
	return true;
}

ssize_t MemoryStream::GetSeekPosition() const
{
	return mCursor;
}

ssize_t MemoryStream::GetFileSize() const
{
	return mBufferSize;
}

void* MemoryStream::GetUnderlyingBuffer() const
{
	return pBuffer;
}

void MemoryStream::Flush()
{
	// No-op.
}

bool MemoryStream::IsAtEnd() const
{
	return mCursor == mBufferSize;
}

bool MemoryStream::Close()
{
	conf_delete(this);
	return true;
}

SE_NAMESPACE_END
//=============================================================================