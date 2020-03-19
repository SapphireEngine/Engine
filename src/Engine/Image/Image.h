#pragma once

#define ALL_MIPLEVELS 127

#include "Platform/FileSystem/FileSystem.h"

//=============================================================================
SE_NAMESPACE_BEGIN

typedef void* (*memoryAllocationFunc)(class Image *image, uint64_t byteCount, uint64_t alignment, void *userData);

typedef enum ImageLoadingResult
{
	IMAGE_LOADING_RESULT_SUCCESS,
	IMAGE_LOADING_RESULT_ALLOCATION_FAILED,
	IMAGE_LOADING_RESULT_DECODING_FAILED
} ImageLoadingResult;

class Image
{
public:
	Image();
	Image(const Image &img);

	void RedefineDimensions(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize = 1);

	unsigned char* GetPixels() const
	{
		return pData;
	}
	unsigned char* GetPixels(const uint32_t mipMapLevel) const;
	unsigned char* GetPixels(unsigned char *dstData, const uint32_t mipMapLevel, const uint32_t dummy);
	unsigned char* GetPixels(const uint32_t mipMapLevel, const uint32_t arraySlice) const;
	size_t GetSizeInBytes() const;

	void SetPixels(unsigned char *pixelData, bool own = false)
	{
		mOwnsMemory = own;
		pData = pixelData;
	}

	void SetPath(const Path *path)
	{
		mLoadFilePath = fsCopyPath(path);
	}

	uint32_t GetBytesPerRow(const uint32_t mipMapLevel = 0) const;
	uint32_t GetRowCount(const uint32_t mipMapLevel = 0) const;
	uint32_t GetWidth() const
	{
		return mWidth;
	}
	uint32_t GetHeight() const
	{
		return mHeight;
	}
	uint32_t GetDepth() const
	{
		return mDepth;
	}
	uint32_t GetWidth(const int mipMapLevel) const;
	uint32_t GetHeight(const int mipMapLevel) const;
	uint32_t GetDepth(const int mipMapLevel) const;
	uint32_t GetMipMapCount() const
	{
		return mMipMapCount;
	}
	const Path* GetPath() const
	{
		return mLoadFilePath;
	}
	uint32_t GetMipMapCountFromDimensions() const;
	uint32_t GetArraySliceSize(const uint32_t mipMapLevel = 0, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;
	uint32_t GetNumberOfPixels(const uint32_t firstMipLevel = 0, uint32_t numMipLevels = ALL_MIPLEVELS) const;
	bool GetColorRange(float &min, float &max);
	TinyImageFormat GetFormat() const
	{
		return mFormat;
	}
	uint32_t GetArrayCount() const
	{
		return mArrayCount;
	}
	uint32_t GetMipMappedSize(const uint32_t firstMipLevel = 0, uint32_t numMipLevels = ALL_MIPLEVELS, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;

	bool Is1D() const
	{
		return (mDepth == 1 && mHeight == 1);
	}
	bool Is2D() const
	{
		return (mDepth == 1 && mHeight > 1);
	}
	bool Is3D() const
	{
		return (mDepth > 1);
	}
	bool IsArray() const
	{
		return (mArrayCount > 1);
	}
	bool IsCube() const
	{
		return (mDepth == 0);
	}
	bool IsSrgb() const
	{
		return TinyImageFormat_IsSRGB(mFormat);
	}
	bool IsLinearLayout() const
	{
		return mLinearLayout;
	}
	bool AreMipsAfterSlices() const
	{
		return mMipsAfterSlices;
	}

	void SetMipsAfterSlices(bool onoff)
	{
		mMipsAfterSlices = onoff;
	}

	uint32_t GetSubtextureAlignment() const
	{
		return max(mSubtextureAlignment, mRowAlignment);
	}
	void SetSubtextureAlignment(uint32_t subtextureAlignment)
	{
		mSubtextureAlignment = subtextureAlignment;
	}

	uint32_t GetRowAlignment() const
	{
		return mRowAlignment;
	}
	void SetRowAlignment(uint32_t rowAlignment)
	{
		mRowAlignment = rowAlignment;
	}

	bool Normalize();
	bool Uncompress(uint32_t newRowAlignment = 1, uint32_t newSubtextureAlignment = 1);
	bool Unpack();

	bool Convert(const TinyImageFormat newFormat);
	bool GenerateMipMaps(const uint32_t mipMaps = ALL_MIPLEVELS);
	
	// Image Format Saving
	bool ISaveDDS(const Path *filePath);
	bool ISaveKTX(const Path *filePath);
	bool ISaveTGA(const Path *filePath);
	bool ISaveBMP(const Path *filePath);
	bool ISavePNG(const Path *filePath);
	bool ISaveHDR(const Path *filePath);
	bool ISaveJPG(const Path *filePath);
	bool ISaveSVT(const Path *filePath, uint32_t pageSize = 128);
	bool Save(const Path *filePath);

	template<typename T>
	inline T GetPixel(uint32_t channel, uint32_t x, uint32_t y, uint32_t z = 0, uint32_t mipLevel = 0, uint32_t slice = 0) const
	{
		SE_ASSERT(IsLinearLayout() && !TinyImageFormat_IsCompressed(mFormat));
		uint32_t channelCount = TinyImageFormat_ChannelCount(mFormat);
		const char* pixels = (const char*)GetPixels(mipLevel, slice);
		uint32_t bytesPerRow = GetBytesPerRow(mipLevel);

		const T* rowPixels = (const T*)(pixels + mHeight * bytesPerRow * z + bytesPerRow * y);
		return rowPixels[x * channelCount + channel];
	}

	template<typename T>
	inline void SetPixel(T pixel, uint32_t channel, uint32_t x, uint32_t y, uint32_t z = 0, uint32_t mipLevel = 0, uint32_t slice = 0)
	{
		SE_ASSERT(IsLinearLayout() && !TinyImageFormat_IsCompressed(mFormat));
		uint32_t channelCount = TinyImageFormat_ChannelCount(mFormat);
		char* pixels = (char*)GetPixels(mipLevel, slice);
		uint32_t bytesPerRow = GetBytesPerRow(mipLevel);

		T* rowPixels = (T*)(pixels + mHeight * bytesPerRow * z + bytesPerRow * y);
		rowPixels[x * channelCount + channel] = pixel;
	}

public:
	typedef ImageLoadingResult(*ImageLoaderFunction)(Image* pImage, FileStream* pStream, memoryAllocationFunc pAllocator, void *pUserData);
	static void AddImageLoader(const char* pExtension, ImageLoaderFunction pFunc);

protected:
	unsigned char *pData;
	PathHandle mLoadFilePath;
	uint32_t mWidth, mHeight, mDepth;
	uint32_t mMipMapCount;
	uint32_t mArrayCount;
	TinyImageFormat mFormat;
	uint32_t mRowAlignment;
	uint32_t mSubtextureAlignment;
	bool mLinearLayout;
	bool mOwnsMemory;
	// is memory (mipmaps*w*h*d)*s or
	// mipmaps * (w*h*d*s) with s being constant for all mipmaps
	bool mMipsAfterSlices;

	static void Init();
	static void Exit();

private:
	void Destroy();

	friend class ResourceLoader;
	friend class AssetPipeline;
	friend bool convertAndSaveImage(const Image& image, bool (Image::*saverFunction)(const Path*), const Path* filePath);
	friend Image* conf_placement_new<Image>(void *ptr);

	// The following Create function will use passed in data as reference without allocating memory for internal pData (meaning the Image object will not own the data)
	unsigned char* Create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize, const unsigned char* rawData, const int rowAlignment = 0, const int subtextureAlignment = 1);

	void Clear();

	//load image
	ImageLoadingResult LoadFromFile(const Path* filePath, memoryAllocationFunc pAllocator = NULL, void* pUserData = NULL, uint32_t rowAlignment = 1, uint32_t subtextureAlignment = 1);
	ImageLoadingResult LoadFromStream(FileStream* pStream, char const* extension, memoryAllocationFunc pAllocator = NULL,void* pUserData = NULL, uint32_t rowAlignment = 0, uint32_t subtextureAlignment = 1);
};

static inline uint32_t calculateMipMapLevels(uint32_t width, uint32_t height)
{
	if ( width == 0 || height == 0 )
		return 0;

	uint32_t result = 1;
	while ( width > 1 || height > 1 )
	{
		width >>= 1;
		height >>= 1;
		result++;
	}
	return result;
}

SE_NAMESPACE_END
//=============================================================================