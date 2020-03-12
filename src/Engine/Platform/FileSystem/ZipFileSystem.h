#pragma once

#include "Platform/FileSystem/FileSystemInternal.h"

//=============================================================================
SE_NAMESPACE_BEGIN

typedef struct zip_t zip_t;

class ZipFileSystem : public FileSystem
{
	struct ZipEntry
	{
		time_t   mTime;
		ssize_t  mSize;
		uint32_t mIndex : 31;
		uint32_t mDirectory : 1;
	};

	zip_t*                           pZipFile;
	Path*                            pPathInParent;
	FileSystemFlags                  mFlags;
	time_t                           mCreationTime;
	stl::string_hash_map<ZipEntry> mEntries;
	time_t                           mLastAccessedTime;

public:
	ZipFileSystem(const Path* pathInParent, zip_t* zipFile, FileSystemFlags flags, time_t creationTime, time_t lastAccessedTime);
	~ZipFileSystem();

	static ZipFileSystem* CreateWithRootAtPath(const Path* rootPath, FileSystemFlags flags);

	Path*  CopyPathInParent() const override;
	char   GetPathDirectorySeparator() const override;
	size_t GetRootPathLength() const override;

	bool   IsReadOnly() const override;
	bool   IsCaseSensitive() const override;

	/// Fills path's buffer with the canonical root path corresponding to the root of absolutePathString,
	/// and returns an offset into absolutePathString containing the path component after the root by pathComponentOffset.
	/// path is assumed to have storage for up to 16 characters.
	bool FormRootPath(const char* absolutePathString, Path* path, size_t* pathComponentOffset) const override;

	time_t GetCreationTime(const Path* filePath) const override;
	time_t GetLastAccessedTime(const Path* filePath) const override;
	time_t GetLastModifiedTime(const Path* filePath) const override;

	bool CreateDirectory(const Path* directoryPath) const override;
	bool DeleteFile(const Path* path) const override;

	bool FileExists(const Path* path) const override;
	bool IsDirectory(const Path* path) const override;

	FileStream* OpenFile(const Path* filePath, FileMode mode) const override;

	bool CopyFile(const Path* sourcePath, const Path* destinationPath, bool overwriteIfExists) const override;
	void EnumerateFilesWithExtension(
		const Path* directory, const char* extension, bool (*processFile)(const Path*, void* userData), void* userData) const override;
	void EnumerateSubDirectories(
		const Path* directory, bool (*processDirectory)(const Path*, void* userData), void* userData) const override;
};

SE_NAMESPACE_END
//=============================================================================