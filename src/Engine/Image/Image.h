#pragma once

#include <tinyimageformat/tinyimageformat_base.h>
#include <tinyimageformat/tinyimageformat_query.h>

#ifndef IMAGE_DISABLE_GOOGLE_BASIS
//Google basis Transcoder
#include <basis_universal/transcoder/basisu_transcoder.h>
#endif
#define ALL_MIPLEVELS 127

#include "Platform/FileSystem/FileSystem.h"

//=============================================================================
SE_NAMESPACE_BEGIN

/************************************************************************************/
// Define some useful macros
#define MCHAR2(a, b) (a | (b << 8))
#define MAKE_CHAR4(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

/*************************************************************************************/

typedef void* (*memoryAllocationFunc)(class Image* pImage, uint64_t byteCount, uint64_t alignment, void* pUserData);

typedef enum ImageLoadingResult
{
	IMAGE_LOADING_RESULT_SUCCESS,
	IMAGE_LOADING_RESULT_ALLOCATION_FAILED,
	IMAGE_LOADING_RESULT_DECODING_FAILED
} ImageLoadingResult;

class Image
{
private:
	void Destroy();

	friend class ResourceLoader;
	friend class AssetPipeline;
	friend bool convertAndSaveImage(const Image& image, bool (Image::*saverFunction)(const Path*), const Path* filePath);
	friend Image* conf_placement_new<Image>(void* ptr);

	// The following Create function will use passed in data as reference without allocating memory for internal pData (meaning the Image object will not own the data)
	unsigned char* Create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize, const unsigned char* rawData, const int rowAlignment = 0, const int subtextureAlignment = 1);

	void Clear();

	//load image
	ImageLoadingResult LoadFromFile(
		const Path* filePath, memoryAllocationFunc pAllocator = NULL, void* pUserData = NULL, uint rowAlignment = 1, uint subtextureAlignment = 1);
	ImageLoadingResult LoadFromStream(
		FileStream* pStream, char const* extension, memoryAllocationFunc pAllocator = NULL,
		void* pUserData = NULL, uint rowAlignment = 0, uint subtextureAlignment = 1);

public:
	Image();
	Image(const Image& img);

	void RedefineDimensions(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize = 1);

	unsigned char* GetPixels() const
	{
		return pData;
	}
	unsigned char* GetPixels(const uint mipMapLevel) const;
	unsigned char* GetPixels(unsigned char* pDstData, const uint mipMapLevel, const uint dummy);
	unsigned char* GetPixels(const uint mipMapLevel, const uint arraySlice) const;
	size_t GetSizeInBytes() const;

	void SetPixels(unsigned char* pixelData, bool own = false)
	{
		mOwnsMemory = own;
		pData = pixelData;
	}

	void SetPath(const Path* path)
	{
		mLoadFilePath = fsCopyPath(path);
	}

	uint				 GetBytesPerRow(const uint mipMapLevel = 0) const;
	uint                 GetRowCount(const uint mipMapLevel = 0) const;
	uint                 GetWidth() const
	{
		return mWidth;
	}
	uint                 GetHeight() const
	{
		return mHeight;
	}
	uint                 GetDepth() const
	{
		return mDepth;
	}
	uint                 GetWidth(const int mipMapLevel) const;
	uint                 GetHeight(const int mipMapLevel) const;
	uint                 GetDepth(const int mipMapLevel) const;
	uint                 GetMipMapCount() const
	{
		return mMipMapCount;
	}
	const Path*          GetPath() const
	{
		return mLoadFilePath;
	}
	uint                 GetMipMapCountFromDimensions() const;
	uint                 GetArraySliceSize(const uint mipMapLevel = 0, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;
	uint                 GetNumberOfPixels(const uint firstMipLevel = 0, uint numMipLevels = ALL_MIPLEVELS) const;
	bool                 GetColorRange(float& min, float& max);
	TinyImageFormat    	 GetFormat() const
	{
		return mFormat;
	}
	uint                 GetArrayCount() const
	{
		return mArrayCount;
	}
	uint                 GetMipMappedSize(
		const uint firstMipLevel = 0, uint numMipLevels = ALL_MIPLEVELS, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;

	bool                 Is1D() const
	{
		return (mDepth == 1 && mHeight == 1);
	}
	bool                 Is2D() const
	{
		return (mDepth == 1 && mHeight > 1);
	}
	bool                 Is3D() const
	{
		return (mDepth > 1);
	}
	bool                 IsArray() const
	{
		return (mArrayCount > 1);
	}
	bool                 IsCube() const
	{
		return (mDepth == 0);
	}
	bool                 IsSrgb() const
	{
		return TinyImageFormat_IsSRGB(mFormat);
	}
	bool                 IsLinearLayout() const
	{
		return mLinearLayout;
	}
	bool                 AreMipsAfterSlices() const
	{
		return mMipsAfterSlices;
	}

	void                 SetMipsAfterSlices(bool onoff)
	{
		mMipsAfterSlices = onoff;
	}

	uint                 GetSubtextureAlignment() const
	{
		return max(mSubtextureAlignment, mRowAlignment);
	}
	void                 SetSubtextureAlignment(uint subtextureAlignment)
	{
		mSubtextureAlignment = subtextureAlignment;
	}

	uint                 GetRowAlignment() const
	{
		return mRowAlignment;
	}
	void                 SetRowAlignment(uint rowAlignment)
	{
		mRowAlignment = rowAlignment;
	}

	bool                 Normalize();
	bool                 Uncompress(uint newRowAlignment = 1, uint newSubtextureAlignment = 1);
	bool                 Unpack();

	bool                 Convert(const TinyImageFormat newFormat);
	bool                 GenerateMipMaps(const uint32_t mipMaps = ALL_MIPLEVELS);

	bool                 iSwap(const int c0, const int c1);

	// Image Format Saving
	bool                 iSaveDDS(const Path* filePath);
	bool                 iSaveKTX(const Path* filePath);
	bool                 iSaveTGA(const Path* filePath);
	bool                 iSaveBMP(const Path* filePath);
	bool                 iSavePNG(const Path* filePath);
	bool                 iSaveHDR(const Path* filePath);
	bool                 iSaveJPG(const Path* filePath);
	bool                 iSaveSVT(const Path* filePath, uint pageSize = 128);
	bool                 Save(const Path* filePath);

	template<typename T>
	inline T GetPixel(uint channel, uint x, uint y, uint z = 0, uint mipLevel = 0, uint slice = 0) const
	{
		ASSERT(IsLinearLayout() && !TinyImageFormat_IsCompressed(mFormat));
		uint channelCount = TinyImageFormat_ChannelCount(mFormat);
		const char* pixels = (const char*)GetPixels(mipLevel, slice);
		uint bytesPerRow = GetBytesPerRow(mipLevel);

		const T* rowPixels = (const T*)(pixels + mHeight * bytesPerRow * z + bytesPerRow * y);
		return rowPixels[x * channelCount + channel];
	}

	template<typename T>
	inline void SetPixel(T pixel, uint channel, uint x, uint y, uint z = 0, uint mipLevel = 0, uint slice = 0)
	{
		ASSERT(IsLinearLayout() && !TinyImageFormat_IsCompressed(mFormat));
		uint channelCount = TinyImageFormat_ChannelCount(mFormat);
		char* pixels = (char*)GetPixels(mipLevel, slice);
		uint bytesPerRow = GetBytesPerRow(mipLevel);

		T* rowPixels = (T*)(pixels + mHeight * bytesPerRow * z + bytesPerRow * y);
		rowPixels[x * channelCount + channel] = pixel;
	}

protected:
	unsigned char*       pData;
	PathHandle           mLoadFilePath;
	uint                 mWidth, mHeight, mDepth;
	uint                 mMipMapCount;
	uint                 mArrayCount;
	TinyImageFormat    	 mFormat;
	uint				 mRowAlignment;
	uint				 mSubtextureAlignment;
	bool                 mLinearLayout;
	bool                 mOwnsMemory;
	// is memory (mipmaps*w*h*d)*s or
	// mipmaps * (w*h*d*s) with s being constant for all mipmaps
	bool				 mMipsAfterSlices;

	static void Init();
	static void Exit();

public:
	typedef ImageLoadingResult(*ImageLoaderFunction)(
		Image* pImage, FileStream* pStream, memoryAllocationFunc pAllocator, void* pUserData);
	static void AddImageLoader(const char* pExtension, ImageLoaderFunction pFunc);
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