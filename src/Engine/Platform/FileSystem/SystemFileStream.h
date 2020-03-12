#pragma once

#include "Platform/FileSystem/FileSystemInternal.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class SystemFileStream : public FileStream
{
	FILE*    pFile;
	FileMode mMode;
	ssize_t  mFileSize;
	bool     mOwnsFile;

public:
	SystemFileStream(FILE* file, FileMode mode, const Path* path, bool ownsFile = true);

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