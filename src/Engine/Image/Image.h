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
	friend class ResourceLoader;
	friend class AssetPipeline;
	friend bool convertAndSaveImage(const Image&, bool (Image::*saverFunction)(const Path*), const Path*);
	friend Image* conf_placement_new<Image>(void*);
public:
	Image();
	Image(const Image &img);

	void RedefineDimensions(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize = 1);

	unsigned char* GetPixels() const
	{
		return p_data;
	}
	unsigned char* GetPixels(const uint32_t mipMapLevel) const;
	unsigned char* GetPixels(unsigned char *dstData, const uint32_t mipMapLevel, const uint32_t dummy);
	unsigned char* GetPixels(const uint32_t mipMapLevel, const uint32_t arraySlice) const;
	size_t GetSizeInBytes() const;

	void SetPixels(unsigned char *pixelData, bool own = false)
	{
		m_ownsMemory = own;
		p_data = pixelData;
	}

	void SetPath(const Path *path)
	{
		m_loadFilePath = fsCopyPath(path);
	}

	uint32_t GetBytesPerRow(const uint32_t mipMapLevel = 0) const;
	uint32_t GetRowCount(const uint32_t mipMapLevel = 0) const;
	uint32_t GetWidth() const
	{
		return m_width;
	}
	uint32_t GetHeight() const
	{
		return m_height;
	}
	uint32_t GetDepth() const
	{
		return m_depth;
	}
	uint32_t GetWidth(const int mipMapLevel) const;
	uint32_t GetHeight(const int mipMapLevel) const;
	uint32_t GetDepth(const int mipMapLevel) const;
	uint32_t GetMipMapCount() const
	{
		return m_mipMapCount;
	}
	const Path* GetPath() const
	{
		return m_loadFilePath;
	}
	uint32_t GetMipMapCountFromDimensions() const;
	uint32_t GetArraySliceSize(const uint32_t mipMapLevel = 0, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;
	uint32_t GetNumberOfPixels(const uint32_t firstMipLevel = 0, uint32_t numMipLevels = ALL_MIPLEVELS) const;
	bool GetColorRange(float &min, float &max);
	TinyImageFormat GetFormat() const
	{
		return m_format;
	}
	uint32_t GetArrayCount() const
	{
		return m_arrayCount;
	}
	uint32_t GetMipMappedSize(const uint32_t firstMipLevel = 0, uint32_t numMipLevels = ALL_MIPLEVELS, TinyImageFormat srcFormat = TinyImageFormat_UNDEFINED) const;

	bool Is1D() const
	{
		return (m_depth == 1 && m_height == 1);
	}
	bool Is2D() const
	{
		return (m_depth == 1 && m_height > 1);
	}
	bool Is3D() const
	{
		return (m_depth > 1);
	}
	bool IsArray() const
	{
		return (m_arrayCount > 1);
	}
	bool IsCube() const
	{
		return (m_depth == 0);
	}
	bool IsSrgb() const
	{
		return TinyImageFormat_IsSRGB(m_format);
	}
	bool IsLinearLayout() const
	{
		return m_linearLayout;
	}
	bool AreMipsAfterSlices() const
	{
		return m_mipsAfterSlices;
	}

	void SetMipsAfterSlices(bool onoff)
	{
		m_mipsAfterSlices = onoff;
	}

	uint32_t GetSubtextureAlignment() const
	{
		return max(m_subtextureAlignment, m_rowAlignment);
	}
	void SetSubtextureAlignment(uint32_t subtextureAlignment)
	{
		m_subtextureAlignment = subtextureAlignment;
	}

	uint32_t GetRowAlignment() const
	{
		return m_rowAlignment;
	}
	void SetRowAlignment(uint32_t rowAlignment)
	{
		m_rowAlignment = rowAlignment;
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
		SE_ASSERT(IsLinearLayout() && !TinyImageFormat_IsCompressed(m_format));
		uint32_t channelCount = TinyImageFormat_ChannelCount(m_format);
		const char *pixels = (const char*)GetPixels(mipLevel, slice);
		uint32_t bytesPerRow = GetBytesPerRow(mipLevel);

		const T *rowPixels = (const T*)(pixels + m_height * bytesPerRow * z + bytesPerRow * y);
		return rowPixels[x * channelCount + channel];
	}

	template<typename T>
	inline void SetPixel(T pixel, uint32_t channel, uint32_t x, uint32_t y, uint32_t z = 0, uint32_t mipLevel = 0, uint32_t slice = 0)
	{
		SE_ASSERT(IsLinearLayout() && !TinyImageFormat_IsCompressed(m_format));
		uint32_t channelCount = TinyImageFormat_ChannelCount(m_format);
		char* pixels = (char*)GetPixels(mipLevel, slice);
		uint32_t bytesPerRow = GetBytesPerRow(mipLevel);

		T* rowPixels = (T*)(pixels + m_height * bytesPerRow * z + bytesPerRow * y);
		rowPixels[x * channelCount + channel] = pixel;
	}

	typedef ImageLoadingResult(*ImageLoaderFunction)(Image *image, FileStream *stream, memoryAllocationFunc allocator, void *userData);
	static void AddImageLoader(const char *extension, ImageLoaderFunction func);

protected:
	static void init();
	static void exit();

	unsigned char *p_data;
	PathHandle m_loadFilePath;
	uint32_t m_width, m_height, m_depth;
	uint32_t m_mipMapCount;
	uint32_t m_arrayCount;
	TinyImageFormat m_format;
	uint32_t m_rowAlignment;
	uint32_t m_subtextureAlignment;
	bool m_linearLayout;
	bool m_ownsMemory;
	// is memory (mipmaps*w*h*d)*s or mipmaps * (w*h*d*s) with s being constant for all mipmaps
	bool m_mipsAfterSlices;

private:
	void destroy();

	// The following create function will use passed in data as reference without allocating memory for internal p_data (meaning the Image object will not own the data)
	unsigned char* create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize, const unsigned char *rawData, const int rowAlignment = 0, const int subtextureAlignment = 1);

	void clear();

	ImageLoadingResult loadFromFile(const Path *filePath, memoryAllocationFunc allocator = nullptr, void *userData = nullptr, uint32_t rowAlignment = 1, uint32_t subtextureAlignment = 1);
	ImageLoadingResult loadFromStream(FileStream *stream, char const *extension, memoryAllocationFunc allocator = nullptr,void *userData = nullptr, uint32_t rowAlignment = 0, uint32_t subtextureAlignment = 1);
};

SE_NAMESPACE_END
//=============================================================================