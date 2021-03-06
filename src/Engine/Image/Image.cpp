#include "stdafx.h"
#include "Image.h"
#include "Core/Debug/Log.h"
#ifndef IMAGE_DISABLE_TINYEXR
#	include <TinyEXR/tinyexr.h>
#endif
#ifndef IMAGE_DISABLE_STB
//stb_image
#	define STB_IMAGE_IMPLEMENTATION
#	define STBI_MALLOC conf_malloc
#	define STBI_REALLOC conf_realloc
#	define STBI_FREE conf_free
#	define STBI_ASSERT SE_ASSERT
#	if defined(__ANDROID__)
#		define STBI_NO_SIMD
#	endif
#	include <Nothings/stb_image.h>
//stb_image_write
#	define STB_IMAGE_WRITE_IMPLEMENTATION
#	define STBIW_MALLOC conf_malloc
#	define STBIW_REALLOC conf_realloc
#	define STBIW_FREE conf_free
#	define STBIW_ASSERT SE_ASSERT
#	include <Nothings/stb_image_write.h>
#	define STB_IMAGE_RESIZE_IMPLEMENTATION
#	include <Nothings/stb_image_resize.h>
#endif
#include <tinyimageformat/tinyimageformat_bits.h>
#include <tinyimageformat/tinyimageformat_decode.h>
#include <tinyimageformat/tinyimageformat_encode.h>
#define TINYDDS_IMPLEMENTATION
#include <tinydds/tinydds.h>
#ifndef IMAGE_DISABLE_KTX
#	define TINYKTX_IMPLEMENTATION
#	include <tinyktx/tinyktx.h>
#endif
#include "ImageHelper.h"
#include "Platform/FileSystem/MemoryStream.h"
#include "Platform/WindowsCore.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
bool convertAndSaveImage(const Image& image, bool (Image::*saverFunction)(const Path*), const Path* filePath)
{
	bool  bSaveImageSuccess = false;
	Image imgCopy(image);
	imgCopy.Uncompress();
	if( imgCopy.Convert(TinyImageFormat_R8G8B8A8_UNORM) )
	{
		bSaveImageSuccess = (imgCopy.*saverFunction)(filePath);
	}

	imgCopy.destroy();
	return bSaveImageSuccess;
}
//-----------------------------------------------------------------------------
static uint32_t getBytesPerRow(uint32_t width, TinyImageFormat sourceFormat, uint32_t alignment)
{
	uint32_t blockWidth = TinyImageFormat_WidthOfBlock(sourceFormat);
	uint32_t blocksPerRow = (width + blockWidth - 1) / blockWidth;
	return round_up(blocksPerRow * TinyImageFormat_BitSizeOfBlock(sourceFormat) / 8, alignment);
}
//-----------------------------------------------------------------------------
// --- BLOCK DECODING ---
//-----------------------------------------------------------------------------
// TODO Decode these decode block don't handle SRGB properly
void decodeColorBlock(unsigned char *dest, int w, int h, int xOff, int yOff, TinyImageFormat format, int red, int blue, unsigned char *src)
{
	unsigned char colors[4][3];

	uint16 c0 = *(uint16*)src;
	uint16 c1 = *(uint16*)(src + 2);

	colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
	colors[0][1] = ((c0 >> 5) & 0x3F) << 2;
	colors[0][2] = (c0 & 0x1F) << 3;

	colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
	colors[1][1] = ((c1 >> 5) & 0x3F) << 2;
	colors[1][2] = (c1 & 0x1F) << 3;

	if ( c0 > c1 || (format == TinyImageFormat_DXBC3_UNORM || format == TinyImageFormat_DXBC3_SRGB) )
	{
		for ( int i = 0; i < 3; i++ )
		{
			colors[2][i] = (2 * colors[0][i] + colors[1][i] + 1) / 3;
			colors[3][i] = (colors[0][i] + 2 * colors[1][i] + 1) / 3;
		}
	}
	else
	{
		for ( int i = 0; i < 3; i++ )
		{
			colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
			colors[3][i] = 0;
		}
	}

	src += 4;
	for ( int y = 0; y < h; y++ )
	{
		unsigned char *dst = dest + yOff * y;
		unsigned int   indexes = src[y];
		for ( int x = 0; x < w; x++ )
		{
			unsigned int index = indexes & 0x3;
			dst[red] = colors[index][0];
			dst[1] = colors[index][1];
			dst[blue] = colors[index][2];
			indexes >>= 2;

			dst += xOff;
		}
	}
}
//-----------------------------------------------------------------------------
void decodeDXT3Block(unsigned char *dest, int w, int h, int xOff, int yOff, unsigned char *src)
{
	for ( int y = 0; y < h; y++ )
	{
		unsigned char *dst = dest + yOff * y;
		unsigned int   alpha = ((unsigned short*)src)[y];
		for ( int x = 0; x < w; x++ )
		{
			*dst = (alpha & 0xF) * 17;
			alpha >>= 4;
			dst += xOff;
		}
	}
}
//-----------------------------------------------------------------------------
void decodeDXT5Block(unsigned char *dest, int w, int h, int xOff, int yOff, unsigned char *src)
{
	unsigned char a0 = src[0];
	unsigned char a1 = src[1];
	uint64_t      alpha = (*(uint64_t*)src) >> 16;

	for ( int y = 0; y < h; y++ )
	{
		unsigned char *dst = dest + yOff * y;
		for ( int x = 0; x < w; x++ )
		{
			int k = ((unsigned int)alpha) & 0x7;
			if ( k == 0 )
			{
				*dst = a0;
			}
			else if ( k == 1 )
			{
				*dst = a1;
			}
			else if ( a0 > a1 )
			{
				*dst = (unsigned char)(((8 - k) * a0 + (k - 1) * a1) / 7);
			}
			else if ( k >= 6 )
			{
				*dst = (k == 6) ? 0 : 255;
			}
			else
			{
				*dst = (unsigned char)(((6 - k) * a0 + (k - 1) * a1) / 5);
			}
			alpha >>= 3;

			dst += xOff;
		}
		if ( w < 4 )
			alpha >>= (3 * (4 - w));
	}
}
//-----------------------------------------------------------------------------
void decodeCompressedImage(unsigned char *dest, unsigned char *src, const int width, const int height, const TinyImageFormat format, uint32_t srcRowPadding, uint32_t dstRowStride)
{
	int sx = (width < 4) ? width : 4;
	int sy = (height < 4) ? height : 4;

	int nChannels = TinyImageFormat_ChannelCount(format);

	for ( int y = 0; y < height; y += 4 )
	{
		for ( int x = 0; x < width; x += 4 )
		{
			unsigned char *dst = dest + y * dstRowStride + x * nChannels;
			if ( format == TinyImageFormat_DXBC2_UNORM || format == TinyImageFormat_DXBC2_SRGB )
			{
				decodeDXT3Block(dst + 3, sx, sy, nChannels, width * nChannels, src);
			}
			else if ( format == TinyImageFormat_DXBC3_UNORM || format == TinyImageFormat_DXBC3_SRGB )
			{
				decodeDXT5Block(dst + 3, sx, sy, nChannels, width * nChannels, src);
			}
			if ( (format == TinyImageFormat_DXBC1_RGBA_UNORM || format == TinyImageFormat_DXBC1_RGB_UNORM) ||
				 (format == TinyImageFormat_DXBC1_RGBA_SRGB || format == TinyImageFormat_DXBC1_RGB_SRGB) )
			{
				decodeColorBlock(dst, sx, sy, nChannels, width * nChannels, format, 0, 2, src);
			}
			else
			{
				if ( format == TinyImageFormat_DXBC4_UNORM || format == TinyImageFormat_DXBC4_SNORM )
				{
					decodeDXT5Block(dst, sx, sy, 1, width, src);
				}
				else if ( format == TinyImageFormat_DXBC5_UNORM || format == TinyImageFormat_DXBC5_SNORM )
				{
					decodeDXT5Block(dst, sx, sy, 2, width * 2, src + 8);
					decodeDXT5Block(dst + 1, sx, sy, 2, width * 2, src);
				}
				else
					return;
			}
			src += TinyImageFormat_BitSizeOfBlock(format) / 8;

		}
		src += srcRowPadding;
	}
}
//-----------------------------------------------------------------------------
Image::Image()
{
#ifndef IMAGE_DISABLE_GOOGLE_BASIS
	// Init basisu
	basist::basisu_transcoder_init();
#endif
}
//-----------------------------------------------------------------------------
Image::Image(const Image &img)
{
	m_width = img.m_width;
	m_height = img.m_height;
	m_depth = img.m_depth;
	m_mipMapCount = img.m_mipMapCount;
	m_arrayCount = img.m_arrayCount;
	m_format = img.m_format;
	m_linearLayout = img.m_linearLayout;
	m_rowAlignment = 1;
	m_subtextureAlignment = 1;

	size_t size = GetSizeInBytes();
	p_data = (unsigned char*)conf_malloc(sizeof(unsigned char) * size);
	memcpy(p_data, img.p_data, size);
	m_loadFilePath = fsCopyPath(img.m_loadFilePath);
}
//-----------------------------------------------------------------------------
void Image::RedefineDimensions(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize)
{
	//Redefine image that was loaded in
	m_format = fmt;
	m_width = w;
	m_height = h;
	m_depth = d;
	m_mipMapCount = mipMapCount;
	m_arrayCount = arraySize;

	switch( m_format )
	{
	case TinyImageFormat_PVRTC1_2BPP_UNORM:
	case TinyImageFormat_PVRTC1_2BPP_SRGB:
	case TinyImageFormat_PVRTC1_4BPP_UNORM:
	case TinyImageFormat_PVRTC1_4BPP_SRGB:
		m_linearLayout = false;
		break;
	default:
		m_linearLayout = true;
	}
}
//-----------------------------------------------------------------------------
unsigned char* Image::GetPixels(const uint32_t mipMapLevel) const
{
	return (mipMapLevel < m_mipMapCount) ? p_data + GetMipMappedSize(0, mipMapLevel) : NULL;
}
//-----------------------------------------------------------------------------
unsigned char* Image::GetPixels(unsigned char *pDstData, const uint32_t mipMapLevel, const uint32_t)
{
	return (mipMapLevel < m_mipMapCount) ? pDstData + GetMipMappedSize(0, mipMapLevel) : NULL;
}
//-----------------------------------------------------------------------------
unsigned char* Image::GetPixels(const uint32_t mipMapLevel, const uint32_t arraySlice) const
{
	if( mipMapLevel >= m_mipMapCount || arraySlice >= m_arrayCount * (IsCube() ? 6 : 1) )
		return nullptr;

	// two ways of storing slices and mipmaps
	// 1. Old Image way. memory slices * ((w*h*d)*mipmaps)
	// 2. Mips after slices way. There are w*h*d*s*mipmaps where slices stays constant(doesn't reduce)
	if( !m_mipsAfterSlices )
	{
		return p_data + GetMipMappedSize(0, m_mipMapCount) * arraySlice + GetMipMappedSize(0, mipMapLevel);
	}
	else
	{
		return p_data + GetMipMappedSize(0, mipMapLevel) + arraySlice * GetArraySliceSize(mipMapLevel);
	}
}
//-----------------------------------------------------------------------------
size_t Image::GetSizeInBytes() const
{
	return GetMipMappedSize(0, m_mipMapCount) * m_arrayCount;
}
//-----------------------------------------------------------------------------
uint32_t Image::GetBytesPerRow(const uint32_t mipMapLevel) const
{
	return getBytesPerRow(GetWidth(mipMapLevel), m_format, m_rowAlignment);
}
//-----------------------------------------------------------------------------
uint32_t Image::GetRowCount(const uint32_t mipMapLevel) const
{
	const uint32_t blockHeight = TinyImageFormat_HeightOfBlock(m_format);
	return (GetHeight(mipMapLevel) + blockHeight - 1) / blockHeight;
}
//-----------------------------------------------------------------------------
uint32_t Image::GetWidth(const int mipMapLevel) const
{
	const uint32_t a = m_width >> mipMapLevel;
	return (a == 0) ? 1 : a;
}
//-----------------------------------------------------------------------------
uint32_t Image::GetHeight(const int mipMapLevel) const
{
	const uint32_t a = m_height >> mipMapLevel;
	return (a == 0) ? 1 : a;
}
//-----------------------------------------------------------------------------
uint32_t Image::GetDepth(const int mipMapLevel) const
{
	const uint32_t a = m_depth >> mipMapLevel;
	return (a == 0) ? 1 : a;
}
//-----------------------------------------------------------------------------
uint32_t Image::GetMipMapCountFromDimensions() const
{
	uint32_t m = max(m_width, m_height);
	m = max(m, m_depth);

	uint32_t i = 0;
	while( m > 0 )
	{
		m >>= 1;
		i++;
	}

	return i;
}
//-----------------------------------------------------------------------------
uint32_t Image::GetArraySliceSize(const uint32_t mipMapLevel, TinyImageFormat srcFormat) const
{
	uint32_t w = GetWidth(mipMapLevel);
	uint32_t d = GetDepth(mipMapLevel);
	if( d == 0 ) d = 1;

	if( srcFormat == TinyImageFormat_UNDEFINED )
		srcFormat = m_format;

	uint32_t bz = TinyImageFormat_DepthOfBlock(srcFormat);

	uint32_t rowCount = GetRowCount(mipMapLevel);
	uint32_t bytesPerRow = getBytesPerRow(w, srcFormat, m_rowAlignment);

	return bytesPerRow * rowCount * ((d + bz - 1) / bz);
}
//-----------------------------------------------------------------------------
uint32_t Image::GetNumberOfPixels(const uint32_t firstMipMapLevel, uint32_t mipMapLevels) const
{
	int w = GetWidth(firstMipMapLevel);
	int h = GetHeight(firstMipMapLevel);
	int d = GetDepth(firstMipMapLevel);
	int size = 0;
	while( mipMapLevels )
	{
		size += w * h * d;
		w >>= 1;
		h >>= 1;
		d >>= 1;
		if( w + h + d == 0 )
			break;
		if( w == 0 )
			w = 1;
		if( h == 0 )
			h = 1;
		if( d == 0 )
			d = 1;

		mipMapLevels--;
	}

	return (m_depth == 0) ? 6 * size : size;
}
//-----------------------------------------------------------------------------
bool Image::GetColorRange(float &min, float &max)
{
	// TODO Deano replace with TinyImageFormat decode calls

	if( TinyImageFormat_IsFloat(m_format) && TinyImageFormat_ChannelBitWidth(m_format, TinyImageFormat_LC_Red) == 32 )
		return false;

	uint32_t nElements = GetNumberOfPixels(0, m_mipMapCount) * TinyImageFormat_ChannelCount(m_format) * m_arrayCount;

	if( nElements <= 0 )
		return false;

	float minVal = FLT_MAX;
	float maxVal = -FLT_MAX;
	for( uint32_t i = 0; i < nElements; i++ )
	{
		float d = ((float*)p_data)[i];
		if( d > maxVal )
			maxVal = d;
		if( d < minVal )
			minVal = d;
	}
	max = maxVal;
	min = minVal;

	return true;
}
//-----------------------------------------------------------------------------
uint32_t Image::GetMipMappedSize(const uint32_t firstMipMapLevel, uint32_t nMipMapLevels, TinyImageFormat srcFormat) const
{
	uint32_t w = GetWidth(firstMipMapLevel);
	uint32_t h = GetHeight(firstMipMapLevel);
	uint32_t d = GetDepth(firstMipMapLevel);
	d = d ? d : 1; // if a cube map treats a 2D texture for calculations
	uint32_t const s = GetArrayCount();
	uint32_t subtextureAlignment = GetSubtextureAlignment();

	if( srcFormat == TinyImageFormat_UNDEFINED )
		srcFormat = m_format;

	if( nMipMapLevels == ALL_MIPLEVELS )
		nMipMapLevels = m_mipMapCount - firstMipMapLevel;

	// PVR formats get special case
	uint64_t const tifname = (TinyImageFormat_Code(m_format) & TinyImageFormat_NAMESPACE_REQUIRED_BITS);
	if( tifname == TinyImageFormat_NAMESPACE_PVRTC )
	{
		// AFAIK pvr isn't supported for arrays
		SE_ASSERT(s == 1);
		uint32_t totalSize = 0;
		uint32_t sizeX = w;
		uint32_t sizeY = h;
		uint32_t sizeD = d;
		int level = nMipMapLevels;

		uint32_t minWidth = 8;
		uint32_t minHeight = 8;
		uint32_t minDepth = 1;
		int bpp = 4;

		if( srcFormat == TinyImageFormat_PVRTC1_2BPP_UNORM || srcFormat == TinyImageFormat_PVRTC1_2BPP_SRGB )
		{
			minWidth = 16;
			minHeight = 8;
			bpp = 2;
		}

		while( level > 0 )
		{
			// If pixel format is compressed, the dimensions need to be padded.
			uint32_t paddedWidth = sizeX + ((-1 * sizeX) % minWidth);
			uint32_t paddedHeight = sizeY + ((-1 * sizeY) % minHeight);
			uint32_t paddedDepth = sizeD + ((-1 * sizeD) % minDepth);

			int mipSize = paddedWidth * paddedHeight * paddedDepth * bpp / 8;

			totalSize += mipSize;

			unsigned int MinimumSize = 1;
			sizeX = max(sizeX / 2, MinimumSize);
			sizeY = max(sizeY / 2, MinimumSize);
			sizeD = max(sizeD / 2, MinimumSize);
			level--;
		}

		return round_up(totalSize, subtextureAlignment);
	}

	uint32_t size = 0;
	for( uint32_t i = 0; i < nMipMapLevels; i += 1 )
	{
		uint32_t by = TinyImageFormat_HeightOfBlock(srcFormat);
		uint32_t bz = TinyImageFormat_DepthOfBlock(srcFormat);

		uint32_t rowCount = (h + by - 1) / by;
		uint32_t bytesPerRow = GetBytesPerRow(i + firstMipMapLevel);

		size += round_up(bytesPerRow * rowCount * ((d + bz - 1) / bz), subtextureAlignment);

		w >>= 1;
		h >>= 1;
		d >>= 1;
		if( w + h + d == 0 )
			break;
		if( w == 0 )
			w = 1;
		if( h == 0 )
			h = 1;
		if( d == 0 )
			d = 1;
	}

	// mips after slices means the slice count is included in mipsize but slices doesn't reduce
	// as slices are included, cubemaps also just fall out
	if( m_mipsAfterSlices ) return (m_depth == 0) ? 6 * size * s : size * s;
	else return (m_depth == 0) ? 6 * size : size;
}
//-----------------------------------------------------------------------------
bool Image::Normalize()
{
	// TODO Deano replace with TinyImageFormat decode calls

	if( TinyImageFormat_IsFloat(m_format) && TinyImageFormat_ChannelBitWidth(m_format, TinyImageFormat_LC_Red) == 32 )
		return false;

	float min, max;
	GetColorRange(min, max);

	const uint32_t elements = GetMipMappedSize();

	float s = 1.0f / (max - min);
	float b = -min * s;
	for( uint32_t i = 0; i < elements; i++ )
	{
		float d = ((float*)p_data)[i];
		((float*)p_data)[i] = d * s + b;
	}

	return true;
}
//-----------------------------------------------------------------------------
bool Image::Uncompress(uint32_t newRowAlignment, uint32_t newSubtextureAlignment)
{
	// only dxtc at the moment
	uint64_t const tifname = (TinyImageFormat_Code(m_format) & TinyImageFormat_NAMESPACE_REQUIRED_BITS);
	if( tifname != TinyImageFormat_NAMESPACE_DXTC )
		return false;

	// only BC 1 to 5 at the moment
	if( m_format == TinyImageFormat_DXBC6H_UFLOAT || m_format == TinyImageFormat_DXBC6H_SFLOAT ||
		m_format == TinyImageFormat_DXBC7_UNORM || m_format == TinyImageFormat_DXBC7_SRGB )
		return false;

	TinyImageFormat destFormat;
	switch( TinyImageFormat_ChannelCount(m_format) )
	{
	case 1: destFormat = TinyImageFormat_R8_UNORM; break;
	case 2: destFormat = TinyImageFormat_R8G8_UNORM; break;
	case 3: destFormat = TinyImageFormat_R8G8B8_UNORM; break;
	case 4: destFormat = TinyImageFormat_R8G8B8A8_UNORM; break;
	default:
		SE_ASSERT(false);
		destFormat = TinyImageFormat_R8_UNORM;
		break;
	}

	uint32_t srcRowAlignment = GetRowAlignment();
	uint32_t srcSubtextureAlignment = GetSubtextureAlignment();
	m_rowAlignment = newRowAlignment;
	m_subtextureAlignment = max(newSubtextureAlignment, newRowAlignment);

	ubyte *newPixels = (ubyte*)conf_malloc(sizeof(ubyte) * GetMipMappedSize(0, m_mipMapCount, destFormat));

	int    level = 0;
	ubyte *src, *dst = newPixels;
	while( (src = GetPixels(level)) != NULL )
	{
		int w = GetWidth(level);
		int h = GetHeight(level);
		int d = (m_depth == 0) ? 6 : GetDepth(level);

		int dstSliceSize = GetArraySliceSize(level, destFormat);
		int srcSliceSize = GetArraySliceSize(level, m_format);

		uint32_t srcRowPadding = getBytesPerRow(w, m_format, srcRowAlignment) - getBytesPerRow(w, m_format, 1);
		uint32_t dstRowStride = GetBytesPerRow(level);

		for( int slice = 0; slice < d; slice++ )
		{
			decodeCompressedImage(dst, src, w, h, m_format, srcRowPadding, dstRowStride);

			dst += round_up(dstSliceSize, m_subtextureAlignment);
			src += round_up(srcSliceSize, srcSubtextureAlignment);
		}
		level++;
	}

	m_format = destFormat;

	destroy();
	p_data = newPixels;

	return true;
}
//-----------------------------------------------------------------------------
bool Image::Unpack()
{
	TinyImageFormat destFormat;
	if( TinyImageFormat_IsFloat(m_format) )
	{
		switch( TinyImageFormat_ChannelCount(m_format) )
		{
		case 1: destFormat = TinyImageFormat_R32_SFLOAT;
			break;
		case 2: destFormat = TinyImageFormat_R32G32_SFLOAT;
			break;
		case 3: destFormat = TinyImageFormat_R32G32B32_SFLOAT;
			break;
		case 4: destFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
			break;
		default: SE_ASSERT(false);
			destFormat = TinyImageFormat_R32_SFLOAT;
			break;
		}
	}
	else if( TinyImageFormat_IsSigned(m_format) )
	{
		switch( TinyImageFormat_ChannelCount(m_format) )
		{
		case 1: destFormat = TinyImageFormat_R8_SNORM;
			break;
		case 2: destFormat = TinyImageFormat_R8G8_SNORM;
			break;
		case 3: destFormat = TinyImageFormat_R8G8B8_SNORM;
			break;
		case 4: destFormat = TinyImageFormat_R8G8B8A8_SNORM;
			break;
		default: SE_ASSERT(false);
			destFormat = TinyImageFormat_R8_SNORM;
			break;
		}
	}
	else if( TinyImageFormat_IsSRGB(m_format) )
	{
		switch( TinyImageFormat_ChannelCount(m_format) )
		{
		case 1: destFormat = TinyImageFormat_R8_SRGB;
			break;
		case 2: destFormat = TinyImageFormat_R8G8_SRGB;
			break;
		case 3: destFormat = TinyImageFormat_R8G8B8_SRGB;
			break;
		case 4: destFormat = TinyImageFormat_R8G8B8A8_SRGB;
			break;
		default: SE_ASSERT(false);
			destFormat = TinyImageFormat_R8_SRGB;
			break;
		}
	}
	else
	{
		switch( TinyImageFormat_ChannelCount(m_format) )
		{
		case 1: destFormat = TinyImageFormat_R8_UNORM;
			break;
		case 2: destFormat = TinyImageFormat_R8G8_UNORM;
			break;
		case 3: destFormat = TinyImageFormat_R8G8B8_UNORM;
			break;
		case 4: destFormat = TinyImageFormat_R8G8B8A8_UNORM;
			break;
		default: SE_ASSERT(false);
			destFormat = TinyImageFormat_R8_UNORM;
			break;
		}
	}

	return Convert(destFormat);
}
//-----------------------------------------------------------------------------
bool Image::Convert(const TinyImageFormat newFormat)
{
	// TODO add RGBE8 to tiny image format
	if( !TinyImageFormat_CanDecodeLogicalPixelsF(m_format) ) return false;
	if( !TinyImageFormat_CanEncodeLogicalPixelsF(newFormat) ) return false;

	int pixelCount = GetNumberOfPixels(0, m_mipMapCount);

	ubyte *newPixels;
	newPixels = (unsigned char*)conf_malloc(GetSizeInBytes());

	TinyImageFormat_DecodeInput input{};
	input.pixel = p_data;
	TinyImageFormat_EncodeOutput output{};
	output.pixel = newPixels;

	float *tmp = (float*)conf_malloc(sizeof(float) * 4 * pixelCount);

	TinyImageFormat_DecodeLogicalPixelsF(m_format, &input, pixelCount, tmp);
	TinyImageFormat_EncodeLogicalPixelsF(newFormat, tmp, pixelCount, &output);

	conf_free(tmp);

	conf_free(p_data);
	p_data = newPixels;
	m_format = newFormat;

	return true;
}
//-----------------------------------------------------------------------------
bool Image::GenerateMipMaps(const uint32_t mipMaps)
{
	if( TinyImageFormat_IsCompressed(m_format) )
		return false;
	if( !(m_width) || !isPowerOf2(m_height) || !isPowerOf2(m_depth) )
		return false;
	if( !m_ownsMemory )
		return false;

	uint32_t actualMipMaps = min(mipMaps, GetMipMapCountFromDimensions());

	if( m_mipMapCount != actualMipMaps )
	{
		uint32_t size = GetMipMappedSize(0, actualMipMaps);
		if( m_arrayCount > 1 )
		{
			ubyte *newPixels = (ubyte*)conf_malloc(sizeof(ubyte) * size * m_arrayCount);

			// Copy top mipmap of all array slices to new location
			int firstMipSize = GetMipMappedSize(0, 1);
			int oldSize = GetMipMappedSize(0, m_mipMapCount);

			for( uint32_t i = 0; i < m_arrayCount; i++ )
			{
				memcpy(newPixels + i * size, p_data + i * oldSize, firstMipSize);
			}

			conf_free(p_data);
			p_data = newPixels;
		}
		else
		{
			p_data = (ubyte*)conf_realloc(p_data, size);
		}
		m_mipMapCount = actualMipMaps;
	}

	int nChannels = TinyImageFormat_ChannelCount(m_format);

	int n = IsCube() ? 6 : 1;

	for( uint32_t arraySlice = 0; arraySlice < m_arrayCount; arraySlice++ )
	{
		for( uint32_t level = 1; level < m_mipMapCount; level++ )
		{
			// TODO: downscale 3D textures using an appropriate filter.

			uint32_t srcWidth = GetWidth(level - 1);
			uint32_t srcHeight = GetHeight(level - 1);
			uint32_t dstWidth = GetWidth(level - 1);
			uint32_t dstHeight = GetHeight(level - 1);

			uint32_t srcSize = GetMipMappedSize(level - 1, 1) / n;
			uint32_t dstSize = GetMipMappedSize(level, 1) / n;

			uint32_t srcStride = GetBytesPerRow(level - 1);
			uint32_t dstStride = GetBytesPerRow(level);

			for( int i = 0; i < n; i++ )
			{
				unsigned char *srcPixels = GetPixels(level - 1, arraySlice) + n * srcSize;
				unsigned char *dstPixels = GetPixels(level, arraySlice) + n * dstSize;

				int8_t physicalChannel = TinyImageFormat_LogicalChannelToPhysical(m_format, TinyImageFormat_LC_Alpha);
				int alphaChannel = physicalChannel;
				if( physicalChannel == TinyImageFormat_PC_CONST_0 || physicalChannel == TinyImageFormat_PC_CONST_1 )
					alphaChannel = STBIR_ALPHA_CHANNEL_NONE;

				// only homogoenous is supported via this method
				// TODO use decode/encode for the others
				// TODO check these methods work for SNORM
				if( TinyImageFormat_IsHomogenous(m_format) )
				{
					uint32 redChanWidth = TinyImageFormat_ChannelBitWidth(m_format, TinyImageFormat_LC_Red);
					if( redChanWidth == 32 && TinyImageFormat_IsFloat(m_format) )
					{
						stbir_resize_float((float*)srcPixels, srcWidth, srcHeight, srcStride,
							(float*)dstPixels, dstWidth, dstHeight, dstStride,
							nChannels);
					}
					else if( redChanWidth == 16 )
					{
						stbir_colorspace colorSpace = TinyImageFormat_IsSRGB(m_format) ? STBIR_COLORSPACE_SRGB : STBIR_COLORSPACE_LINEAR;
						stbir_resize_uint16_generic((uint16_t*)srcPixels, srcWidth, srcHeight, srcStride,
							(uint16_t*)dstPixels, dstWidth, dstHeight, dstStride,
							nChannels, alphaChannel, 0,
							STBIR_EDGE_WRAP, STBIR_FILTER_DEFAULT, colorSpace, NULL);
					}
					else if( redChanWidth == 8 )
					{

						if( TinyImageFormat_IsSRGB(m_format) )
							stbir_resize_uint8_srgb(srcPixels, srcWidth, srcHeight, srcStride,
								dstPixels, dstWidth, dstHeight, dstStride,
								nChannels, alphaChannel, 0);
						else
							stbir_resize_uint8(srcPixels, srcWidth, srcHeight, srcStride,
								dstPixels, dstWidth, dstHeight, dstStride,
								nChannels);
					}
					// TODO: fall back to to be written generic downsizer
				}
			}
		}
	}

	return true;
}
//-----------------------------------------------------------------------------
static void tinyktxddsCallbackError(void *user, char const *msg)
{
	LOGF(LogLevel::eERROR, "Tiny_ ERROR: %s", msg);
}
//-----------------------------------------------------------------------------
static void *tinyktxddsCallbackAlloc(void *user, size_t size)
{
	return conf_malloc(size);
}
//-----------------------------------------------------------------------------
static void tinyktxddsCallbackFree(void *user, void *data)
{
	conf_free(data);
}
//-----------------------------------------------------------------------------
static size_t tinyktxddsCallbackRead(void *user, void* data, size_t size)
{
	FileStream* file = (FileStream*)user;
	return fsReadFromStream(file, data, size);
}
//-----------------------------------------------------------------------------
static void tinyktxddsCallbackWrite(void* user, void const* data, size_t size)
{
	FileStream* file = (FileStream*)user;
	fsWriteToStream(file, data, size);
}
//-----------------------------------------------------------------------------
bool Image::SaveDDS(const Path *filePath)
{
	TinyDDS_WriteCallbacks callback
	{
		&tinyktxddsCallbackError,
		&tinyktxddsCallbackAlloc,
		&tinyktxddsCallbackFree,
		&tinyktxddsCallbackWrite,
	};

	TinyDDS_Format fmt = TinyImageFormat_ToTinyDDSFormat(m_format);
#ifndef IMAGE_DISABLE_KTX
	if( fmt == TDDS_UNDEFINED )
		return convertAndSaveImage(*this, &Image::SaveKTX, filePath);
#endif

	FileStream *fh = fsOpenFile(filePath, FM_WRITE_BINARY);
	if( !fh )
		return false;

	uint32_t mipmapsizes[TINYDDS_MAX_MIPMAPLEVELS];
	void const *mipmaps[TINYDDS_MAX_MIPMAPLEVELS];
	memset(mipmapsizes, 0, sizeof(uint32_t) * TINYDDS_MAX_MIPMAPLEVELS);
	memset(mipmaps, 0, sizeof(void const*) * TINYDDS_MAX_MIPMAPLEVELS);

	for( unsigned int i = 0; i < m_mipMapCount; ++i )
	{
		mipmapsizes[i] = (uint32_t)ImageGetMipMappedSize(m_width, m_height, m_depth, m_mipMapCount, m_format);
		mipmaps[i] = GetPixels(i);
	}

	bool result = TinyDDS_WriteImage(&callback, fh, m_width, m_height, m_depth, m_arrayCount, m_mipMapCount, fmt, m_depth == 0, true, mipmapsizes, mipmaps);
	fsCloseStream(fh);
	return result;
}
//-----------------------------------------------------------------------------
#ifndef IMAGE_DISABLE_KTX
bool Image::SaveKTX(const Path *filePath)
{
	TinyKtx_WriteCallbacks callback
	{
		&tinyktxddsCallbackError,
		&tinyktxddsCallbackAlloc,
		&tinyktxddsCallbackFree,
		&tinyktxddsCallbackWrite,
	};

	if( getBytesPerRow(m_width, m_format, m_rowAlignment) != getBytesPerRow(m_width, m_format, 1) )
	{
		LOGF(LogLevel::eWARNING, "Saving KTX images with a padded row stride is unimplemented.");
		return false; // TODO: handle padded rows.
	}

	TinyKtx_Format fmt = TinyImageFormat_ToTinyKtxFormat(m_format);
	if( fmt == TKTX_UNDEFINED )
		return convertAndSaveImage(*this, &Image::SaveKTX, filePath);

	FileStream* fh = fsOpenFile(filePath, FM_WRITE_BINARY);

	if( !fh )
		return false;

	uint32_t mipmapsizes[TINYKTX_MAX_MIPMAPLEVELS];
	void const *mipmaps[TINYKTX_MAX_MIPMAPLEVELS];
	memset(mipmapsizes, 0, sizeof(uint32_t) * TINYKTX_MAX_MIPMAPLEVELS);
	memset(mipmaps, 0, sizeof(void const*) * TINYKTX_MAX_MIPMAPLEVELS);

	for( unsigned int i = 0; i < m_mipMapCount; ++i )
	{
		mipmapsizes[i] = (uint32_t)ImageGetMipMappedSize(m_width, m_height, m_depth, m_mipMapCount, m_format);
		mipmaps[i] = GetPixels(i);
	}

	bool result = TinyKtx_WriteImage(&callback, fh, m_width, m_height, m_depth, m_arrayCount, m_mipMapCount, fmt, m_depth == 0, mipmapsizes, mipmaps);

	fsCloseStream(fh);
	return result;
}
#endif
//-----------------------------------------------------------------------------
#ifndef IMAGE_DISABLE_STB
bool Image::SaveTGA(const Path *filePath)
{
	// TODO: should use stbi_write_x_to_func methods rather than relying on the path being a disk path.
	if( getBytesPerRow(m_width, m_format, m_rowAlignment) != getBytesPerRow(m_width, m_format, 1) )
	{
		LOGF(LogLevel::eWARNING, "Saving TGA images with a padded row stride is unimplemented.");
		return false; // TODO: handle padded rows.
	}

	const char *fileName = fsGetPathAsNativeString(filePath);
	switch( m_format )
	{
	case TinyImageFormat_R8_UNORM: return 0 != stbi_write_tga(fileName, m_width, m_height, 1, p_data); break;
	case TinyImageFormat_R8G8_UNORM: return 0 != stbi_write_tga(fileName, m_width, m_height, 2, p_data); break;
	case TinyImageFormat_R8G8B8_UNORM: return 0 != stbi_write_tga(fileName, m_width, m_height, 3, p_data); break;
	case TinyImageFormat_R8G8B8A8_UNORM: return 0 != stbi_write_tga(fileName, m_width, m_height, 4, p_data); break;
	default:
		// uncompress/convert and try again
		return convertAndSaveImage(*this, &Image::SaveTGA, filePath);
	}
	//return false; //Unreachable
}
//-----------------------------------------------------------------------------
bool Image::SaveBMP(const Path *filePath)
{
	// TODO: should use stbi_write_x_to_func methods rather than relying on the path being a disk path.

	if( getBytesPerRow(m_width, m_format, m_rowAlignment) != getBytesPerRow(m_width, m_format, 1) )
	{
		LOGF(LogLevel::eWARNING, "Saving BMP images with a padded row stride is unimplemented.");
		return false; // TODO: handle padded rows.
	}

	const char *fileName = fsGetPathAsNativeString(filePath);
	switch( m_format )
	{
	case TinyImageFormat_R8_UNORM: stbi_write_bmp(fileName, m_width, m_height, 1, p_data); break;
	case TinyImageFormat_R8G8_UNORM: stbi_write_bmp(fileName, m_width, m_height, 2, p_data); break;
	case TinyImageFormat_R8G8B8_UNORM: stbi_write_bmp(fileName, m_width, m_height, 3, p_data); break;
	case TinyImageFormat_R8G8B8A8_UNORM: stbi_write_bmp(fileName, m_width, m_height, 4, p_data); break;
	default:
		// uncompress/convert and try again
		return convertAndSaveImage(*this, &Image::SaveBMP, filePath);
	}
	return true;
}
//-----------------------------------------------------------------------------
bool Image::SavePNG(const Path *filePath)
{
	// TODO: should use stbi_write_x_to_func methods rather than relying on the path being a disk path.
	const char *fileName = fsGetPathAsNativeString(filePath);
	switch( m_format )
	{
	case TinyImageFormat_R8_UNORM: stbi_write_png(fileName, m_width, m_height, 1, p_data, GetBytesPerRow()); break;
	case TinyImageFormat_R8G8_UNORM: stbi_write_png(fileName, m_width, m_height, 2, p_data, GetBytesPerRow()); break;
	case TinyImageFormat_R8G8B8_UNORM: stbi_write_png(fileName, m_width, m_height, 3, p_data, GetBytesPerRow()); break;
	case TinyImageFormat_R8G8B8A8_UNORM: stbi_write_png(fileName, m_width, m_height, 4, p_data, GetBytesPerRow()); break;
	default:
	{
		// uncompress/convert and try again
		return convertAndSaveImage(*this, &Image::SavePNG, filePath);
	}
	}

	return true;
}
//-----------------------------------------------------------------------------
bool Image::SaveHDR(const Path *filePath)
{
	// TODO: should use stbi_write_x_to_func methods rather than relying on the path being a disk path.

	if( getBytesPerRow(m_width, m_format, m_rowAlignment) != getBytesPerRow(m_width, m_format, 1) )
	{
		LOGF(LogLevel::eWARNING, "Saving HDR images with a padded row stride is unimplemented.");
		return false; // TODO: handle padded rows.
	}

	const char *fileName = fsGetPathAsNativeString(filePath);
	switch( m_format )
	{
	case TinyImageFormat_R32_SFLOAT: stbi_write_hdr(fileName, m_width, m_height, 1, (float*)p_data); break;
	case TinyImageFormat_R32G32_SFLOAT: stbi_write_hdr(fileName, m_width, m_height, 2, (float*)p_data); break;
	case TinyImageFormat_R32G32B32_SFLOAT: stbi_write_hdr(fileName, m_width, m_height, 3, (float*)p_data); break;
	case TinyImageFormat_R32G32B32A32_SFLOAT: stbi_write_hdr(fileName, m_width, m_height, 4, (float*)p_data); break;
	default:
	{
		// uncompress/convert and try again
		return convertAndSaveImage(*this, &Image::SaveHDR, filePath);
	}
	}

	return true;
}
//-----------------------------------------------------------------------------
bool Image::SaveJPG(const Path *filePath)
{
	// TODO: should use stbi_write_x_to_func methods rather than relying on the path being a disk path.

	if( getBytesPerRow(m_width, m_format, m_rowAlignment) != getBytesPerRow(m_width, m_format, 1) )
	{
		LOGF(LogLevel::eWARNING, "Saving JPEG images with a padded row stride is unimplemented.");
		return false; // TODO: handle padded rows.
	}

	const char *fileName = fsGetPathAsNativeString(filePath);
	switch( m_format )
	{
	case TinyImageFormat_R8_UNORM: stbi_write_jpg(fileName, m_width, m_height, 1, p_data, 0); break;
	case TinyImageFormat_R8G8_UNORM: stbi_write_jpg(fileName, m_width, m_height, 2, p_data, 0); break;
	case TinyImageFormat_R8G8B8_UNORM: stbi_write_jpg(fileName, m_width, m_height, 3, p_data, 0); break;
	case TinyImageFormat_R8G8B8A8_UNORM: stbi_write_jpg(fileName, m_width, m_height, 4, p_data, 0); break;
	default:
	{
		// uncompress/convert and try again
		return convertAndSaveImage(*this, &Image::SaveJPG, filePath);
	}
	}

	return true;
}
#endif
//-----------------------------------------------------------------------------
bool Image::SaveSVT(const Path *filePath, uint32_t pageSize)
{
	if( m_format != TinyImageFormat::TinyImageFormat_R8G8B8A8_UNORM )
	{
		// uncompress/convert
		if( !Convert(TinyImageFormat::TinyImageFormat_R8G8B8A8_UNORM) )
			return false;
	}

	if( m_mipMapCount == 1 )
	{
		if( !GenerateMipMaps((uint32_t)log2f((float)min(m_width, m_height))) )
			return false;
	}

	FileStream *fh = fsOpenFile(filePath, FM_WRITE_BINARY);
	if( !fh )
		return false;

	//TODO: SVT should support any components somepoint
	const uint32_t numberOfComponents = 4;

	//Header

	fsWriteToStream(fh, &m_width, sizeof(uint32_t));
	fsWriteToStream(fh, &m_height, sizeof(uint32_t));

	//mip count
	fsWriteToStream(fh, &m_mipMapCount, sizeof(uint32_t));

	//Page size
	fsWriteToStream(fh, &pageSize, sizeof(uint32_t));

	//number of components
	fsWriteToStream(fh, &numberOfComponents, sizeof(uint32_t));

	uint32_t mipPageCount = m_mipMapCount - (uint32_t)log2f((float)pageSize);

	// Allocate Pages
	unsigned char** mipLevelPixels = (unsigned char**)conf_calloc(m_mipMapCount, sizeof(unsigned char*));
	unsigned char*** pagePixels = (unsigned char***)conf_calloc(mipPageCount + 1, sizeof(unsigned char**));

	uint32_t *mipSizes = (uint32_t*)conf_calloc(m_mipMapCount, sizeof(uint32_t));

	for( uint32_t i = 0; i < m_mipMapCount; ++i )
	{
		uint32_t mipSize = (m_width >> i) * (m_height >> i) * numberOfComponents;
		mipSizes[i] = mipSize;
		mipLevelPixels[i] = (unsigned char*)conf_calloc(mipSize, sizeof(unsigned char));
		memcpy(mipLevelPixels[i], GetPixels(i), mipSize * sizeof(unsigned char));
	}

	// Store Mip data
	for( uint32_t i = 0; i < mipPageCount; ++i )
	{
		uint32_t xOffset = m_width >> i;
		uint32_t yOffset = m_height >> i;

		// width and height in tiles
		uint32_t tileWidth = xOffset / pageSize;
		uint32_t tileHeight = yOffset / pageSize;

		uint32_t xMipOffset = 0;
		uint32_t yMipOffset = 0;
		uint32_t pageIndex = 0;

		uint32_t rowLength = xOffset * numberOfComponents;

		pagePixels[i] = (unsigned char**)conf_calloc(tileWidth * tileHeight, sizeof(unsigned char*));

		for( uint32_t j = 0; j < tileHeight; ++j )
		{
			for( uint32_t k = 0; k < tileWidth; ++k )
			{
				pagePixels[i][pageIndex] = (unsigned char*)conf_calloc(pageSize * pageSize, sizeof(unsigned char) * numberOfComponents);

				for( uint32_t y = 0; y < pageSize; ++y )
				{
					for( uint32_t x = 0; x < pageSize; ++x )
					{
						uint32_t mipPageIndex = (y * pageSize + x) * numberOfComponents;
						uint32_t mipIndex = rowLength * (y + yMipOffset) + (numberOfComponents) * (x + xMipOffset);

						pagePixels[i][pageIndex][mipPageIndex + 0] = mipLevelPixels[i][mipIndex + 0];
						pagePixels[i][pageIndex][mipPageIndex + 1] = mipLevelPixels[i][mipIndex + 1];
						pagePixels[i][pageIndex][mipPageIndex + 2] = mipLevelPixels[i][mipIndex + 2];
						pagePixels[i][pageIndex][mipPageIndex + 3] = mipLevelPixels[i][mipIndex + 3];
					}
				}

				xMipOffset += pageSize;
				pageIndex += 1;
			}

			xMipOffset = 0;
			yMipOffset += pageSize;
		}
	}

	uint32_t mipTailPageSize = 0;

	pagePixels[mipPageCount] = (unsigned char**)conf_calloc(1, sizeof(unsigned char*));

	// Calculate mip tail size
	for( uint32_t i = mipPageCount; i < m_mipMapCount - 1; ++i )
	{
		uint32_t mipSize = mipSizes[i];
		mipTailPageSize += mipSize;
	}

	pagePixels[mipPageCount][0] = (unsigned char*)conf_calloc(mipTailPageSize, sizeof(unsigned char));

	// Store mip tail data
	uint32_t mipTailPageWrites = 0;
	for( uint32_t i = mipPageCount; i < m_mipMapCount - 1; ++i )
	{
		uint32_t mipSize = mipSizes[i];

		for( uint32_t j = 0; j < mipSize; ++j )
		{
			pagePixels[mipPageCount][0][mipTailPageWrites++] = mipLevelPixels[i][j];
		}
	}

	// Write mip data
	for( uint32_t i = 0; i < mipPageCount; ++i )
	{
		// width and height in tiles
		uint32_t mipWidth = (m_width >> i) / pageSize;
		uint32_t mipHeight = (m_height >> i) / pageSize;

		for( uint32_t j = 0; j < mipWidth * mipHeight; ++j )
		{
			fsWriteToStream(fh, pagePixels[i][j], pageSize * pageSize * numberOfComponents * sizeof(char));
		}
	}

	// Write mip tail data
	fsWriteToStream(fh, pagePixels[mipPageCount][0], mipTailPageSize * sizeof(char));

	// free memory
	conf_free(mipSizes);

	for( uint32_t i = 0; i < mipPageCount; ++i )
	{
		// width and height in tiles
		uint32_t mipWidth = (m_width >> i) / pageSize;
		uint32_t mipHeight = (m_height >> i) / pageSize;
		uint32_t pageIndex = 0;

		for( uint32_t j = 0; j < mipHeight; ++j )
		{
			for( uint32_t k = 0; k < mipWidth; ++k )
			{
				conf_free(pagePixels[i][pageIndex]);
				pageIndex += 1;
			}
		}

		conf_free(pagePixels[i]);
	}
	conf_free(pagePixels[mipPageCount][0]);
	conf_free(pagePixels[mipPageCount]);
	conf_free(pagePixels);

	for( uint32_t i = 0; i < m_mipMapCount; ++i )
	{
		conf_free(mipLevelPixels[i]);
	}
	conf_free(mipLevelPixels);

	fsCloseStream(fh);

	return true;
}
//-----------------------------------------------------------------------------
struct ImageSaverDefinition
{
	typedef bool (Image::*ImageSaverFunction)(const Path *filePath);
	const char *Extension;
	ImageSaverFunction Loader;
};
//-----------------------------------------------------------------------------
static ImageSaverDefinition gImageSavers[] =
{
#ifndef IMAGE_DISABLE_STB
	{ ".bmp", &Image::SaveBMP },
	{ ".hdr", &Image::SaveHDR },
	{ ".png", &Image::SavePNG },
	{ ".tga", &Image::SaveTGA },
	{ ".jpg", &Image::SaveJPG },
#endif
	{ ".dds", &Image::SaveDDS },
#ifndef IMAGE_DISABLE_KTX
	{ ".ktx", &Image::SaveKTX }
#endif
};
//-----------------------------------------------------------------------------
bool Image::Save(const Path *filePath)
{
	char extension[16];
	fsGetLowercasedPathExtension(filePath, extension, 16);
	bool support = false;

	for( int i = 0; i < sizeof(gImageSavers) / sizeof(gImageSavers[0]); i++ )
	{
		if( stricmp(extension, gImageSavers[i].Extension) == 0 )
		{
			support = true;
			return (this->*gImageSavers[i].Loader)(filePath);
		}
	}
	if( !support )
	{
		LOGF(LogLevel::eERROR, "Can't save this file format for image  :  %s", fsGetPathAsNativeString(filePath));
	}

	return false;
}
//-----------------------------------------------------------------------------
// Image loading
// struct of table for file format to loading function
struct ImageLoaderDefinition
{
	char const *Extension;
	Image::ImageLoaderFunction Loader;
};
#define MAX_IMAGE_LOADERS 12
static ImageLoaderDefinition gImageLoaders[MAX_IMAGE_LOADERS];
uint32_t gImageLoaderCount = 0;
//-----------------------------------------------------------------------------
void Image::AddImageLoader(const char *extension, ImageLoaderFunction func)
{
	gImageLoaders[gImageLoaderCount++] = { extension, func };
}
//-----------------------------------------------------------------------------
#ifndef IMAGE_DISABLE_STB
ImageLoadingResult loadSTBFromStream(Image *image, FileStream *stream, MemoryAllocationFunc allocator, void *userData)
{
	if( stream == nullptr || fsGetStreamFileSize(stream) == 0 )
		return IMAGE_LOADING_RESULT_DECODING_FAILED;

	LOGF(LogLevel::eWARNING, "Image %s is not in a GPU-compressed format; please use a GPU-compressed format for best performance.", fsGetPathAsNativeString(image->GetPath()));

	int width, height, components;

	size_t memSize = fsGetStreamFileSize(stream);
	void *memory = fsGetStreamBufferIfPresent(stream);
	stbi_uc *imageBytes = nullptr;

	if( memory )
	{
		imageBytes = stbi_load_from_memory((stbi_uc*)memory, (int)memSize, &width, &height, &components, 0);
	}
	else
	{
		memory = conf_malloc(memSize);
		fsReadFromStream(stream, memory, memSize);
		imageBytes = stbi_load_from_memory((stbi_uc*)memory, (int)memSize, &width, &height, &components, 0);
		conf_free(memory);
	}

	if( !imageBytes )
	{
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	TinyImageFormat format;
	switch( components )
	{
	case 1:
		format = TinyImageFormat_R8_UNORM;
		break;
	case 2:
		format = TinyImageFormat_R8G8_UNORM;
		break;
	default:
		format = TinyImageFormat_R8G8B8A8_UNORM;
		break;
	}

	image->RedefineDimensions(format, width, height, 1, 1, 1);

	size_t destinationBytesPerRow = image->GetBytesPerRow();
	size_t imageByteSize = image->GetSizeInBytes();

	if( allocator )
		image->SetPixels((unsigned char*)allocator(image, imageByteSize, image->GetSubtextureAlignment(), userData));
	else
		image->SetPixels((unsigned char*)conf_malloc(imageByteSize), true);

	if( !image->GetPixels() )
		return IMAGE_LOADING_RESULT_ALLOCATION_FAILED;

	unsigned char* dstPixels = image->GetPixels(0, 0);
	if( components == 3 )
	{
		size_t sourceBytesPerRow = 3 * width;
		for( uint32_t y = 0; y < (uint32_t)height; y += 1 )
		{
			unsigned char* srcRow = imageBytes + y * sourceBytesPerRow;
			unsigned char* dstRow = dstPixels + y * destinationBytesPerRow;

			for( uint32_t x = 0; x < (uint32_t)width; x += 1 )
			{
				dstRow[4 * x + 0] = srcRow[3 * x + 0];
				dstRow[4 * x + 1] = srcRow[3 * x + 1];
				dstRow[4 * x + 2] = srcRow[3 * x + 2];
				dstRow[4 * x + 3] = ~0;
			}
		}
	}
	else
	{
		size_t sourceBytesPerRow = TinyImageFormat_BitSizeOfBlock(format) / 8 * width;
		for( uint32_t y = 0; y < (uint32_t)height; y += 1 )
		{
			unsigned char* srcRow = imageBytes + y * sourceBytesPerRow;
			unsigned char* dstRow = dstPixels + y * destinationBytesPerRow;
			memcpy(dstRow, srcRow, sourceBytesPerRow);
		}
	}

	stbi_image_free(imageBytes);

	return IMAGE_LOADING_RESULT_SUCCESS;
}
#endif
//-----------------------------------------------------------------------------
static bool tinyktxddsCallbackSeek(void *user, int64_t offset)
{
	FileStream *file = (FileStream*)user;
	return fsSeekStream(file, SBO_START_OF_FILE, offset);

}
//-----------------------------------------------------------------------------
static int64_t tinyktxddsCallbackTell(void *user)
{
	FileStream *file = (FileStream*)user;
	return fsGetStreamSeekPosition(file);
}
//-----------------------------------------------------------------------------
ImageLoadingResult loadDDSFromStream(Image *image, FileStream *stream, MemoryAllocationFunc allocator, void *userData)
{
	if( stream == nullptr || fsGetStreamFileSize(stream) == 0 )
		return IMAGE_LOADING_RESULT_DECODING_FAILED;

	TinyDDS_Callbacks callbacks
	{
		&tinyktxddsCallbackError,
		&tinyktxddsCallbackAlloc,
		&tinyktxddsCallbackFree,
		tinyktxddsCallbackRead,
		&tinyktxddsCallbackSeek,
		&tinyktxddsCallbackTell
	};

	auto ctx = TinyDDS_CreateContext(&callbacks, (void*)stream);
	bool headerOkay = TinyDDS_ReadHeader(ctx);
	if( !headerOkay )
	{
		TinyDDS_DestroyContext(ctx);
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	uint32_t w = TinyDDS_Width(ctx);
	uint32_t h = TinyDDS_Height(ctx);
	uint32_t d = TinyDDS_Depth(ctx);
	uint32_t s = TinyDDS_ArraySlices(ctx);
	uint32_t mm = TinyDDS_NumberOfMipmaps(ctx);
	TinyImageFormat fmt = TinyImageFormat_FromTinyDDSFormat(TinyDDS_GetFormat(ctx));
	if( fmt == TinyImageFormat_UNDEFINED )
	{
		TinyDDS_DestroyContext(ctx);
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	// TheForge Image uses d = 0 as cubemap marker
	if( TinyDDS_IsCubemap(ctx) )
		d = 0;
	else
		d = d ? d : 1;
	s = s ? s : 1;

	image->RedefineDimensions(fmt, w, h, d, mm, s);
	image->SetMipsAfterSlices(true); // tinyDDS API is mips after slices even if DDS traditionally weren't

	if( d == 0 )
		s *= 6;

	size_t size = image->GetSizeInBytes();

	if( allocator )
		image->SetPixels((unsigned char*)allocator(image, size, image->GetSubtextureAlignment(), userData));
	else
		image->SetPixels((unsigned char*)conf_malloc(sizeof(unsigned char) * size), true);

	if( !image->GetPixels() )
	{
		TinyDDS_DestroyContext(ctx);
		return IMAGE_LOADING_RESULT_ALLOCATION_FAILED;
	}

	for( uint32_t mipMapLevel = 0; mipMapLevel < image->GetMipMapCount(); mipMapLevel++ )
	{
		size_t blocksPerRow = (w + TinyImageFormat_WidthOfBlock(fmt) - 1) / TinyImageFormat_WidthOfBlock(fmt);
		size_t sourceBytesPerRow = TinyImageFormat_BitSizeOfBlock(fmt) / 8 * blocksPerRow;
		size_t destinationBytesPerRow = image->GetBytesPerRow(mipMapLevel);
		size_t rowCount = (h + TinyImageFormat_HeightOfBlock(fmt) - 1) / TinyImageFormat_HeightOfBlock(fmt);
		size_t depthCount = d == 0 ? 1 : (TinyDDS_IsCubemap(ctx) ? 1 : d);

		size_t const expectedSize = sourceBytesPerRow * rowCount * depthCount * s;
		size_t const fileSize = TinyDDS_ImageSize(ctx, mipMapLevel);
		if( expectedSize != fileSize )
		{
			LOGF(LogLevel::eERROR, "DDS file %s mipmap %i size error %liu < %liu", fsGetPathAsNativeString(image->GetPath()), mipMapLevel, expectedSize, fileSize);
			return IMAGE_LOADING_RESULT_DECODING_FAILED;
		}

		const unsigned char* src = (const unsigned char*)TinyDDS_ImageRawData(ctx, mipMapLevel);

		for( uint32_t slice = 0; slice < s; slice += 1 )
		{
			unsigned char *dst = image->GetPixels(mipMapLevel, slice);
			for( uint32_t row = 0; row < rowCount * depthCount; row += 1 )
			{
				const unsigned char* srcRow = src + (slice * rowCount + row) * sourceBytesPerRow;
				unsigned char* dstRow = dst + row * destinationBytesPerRow;
				memcpy(dstRow, srcRow, sourceBytesPerRow);
			}
		}

		w /= 2;
		h /= 2;
		d /= 2;
	}

	TinyDDS_DestroyContext(ctx);

	return IMAGE_LOADING_RESULT_SUCCESS;
}
//-----------------------------------------------------------------------------
#ifndef IMAGE_DISABLE_KTX
ImageLoadingResult loadKTXFromStream(Image *image, FileStream *stream, MemoryAllocationFunc allocator, void *userData)
{
	TinyKtx_Callbacks callbacks
	{
		&tinyktxddsCallbackError,
		&tinyktxddsCallbackAlloc,
		&tinyktxddsCallbackFree,
		&tinyktxddsCallbackRead,
		&tinyktxddsCallbackSeek,
		&tinyktxddsCallbackTell
	};

	auto ctx = TinyKtx_CreateContext(&callbacks, (void*)stream);
	bool headerOkay = TinyKtx_ReadHeader(ctx);
	if( !headerOkay )
	{
		TinyKtx_DestroyContext(ctx);
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	uint32_t w = TinyKtx_Width(ctx);
	uint32_t h = TinyKtx_Height(ctx);
	uint32_t d = TinyKtx_Depth(ctx);
	uint32_t s = TinyKtx_ArraySlices(ctx);
	uint32_t mm = TinyKtx_NumberOfMipmaps(ctx);
	TinyImageFormat fmt = TinyImageFormat_FromTinyKtxFormat(TinyKtx_GetFormat(ctx));
	if( fmt == TinyImageFormat_UNDEFINED )
	{
		TinyKtx_DestroyContext(ctx);
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	// TheForge Image uses d = 0 as cubemap marker
	if( TinyKtx_IsCubemap(ctx) ) d = 0;
	else d = d ? d : 1;
	s = s ? s : 1;

	if( d == 0 )
		s *= 6;

	image->RedefineDimensions(fmt, w, h, d, mm, s);
	image->SetMipsAfterSlices(true); // tinyDDS API is mips after slices even if DDS traditionally weren't

	size_t size = image->GetSizeInBytes();

	if( allocator )
		image->SetPixels((uint8_t*)allocator(image, size, image->GetSubtextureAlignment(), userData));
	else
		image->SetPixels((uint8_t*)conf_malloc(sizeof(uint8_t) * size), true);

	if( !image->GetPixels() )
	{
		TinyKtx_DestroyContext(ctx);
		return IMAGE_LOADING_RESULT_ALLOCATION_FAILED;
	}

	for( uint32_t mipMapLevel = 0; mipMapLevel < image->GetMipMapCount(); mipMapLevel++ )
	{
		uint32_t blocksPerRow = (w + TinyImageFormat_WidthOfBlock(fmt) - 1) / TinyImageFormat_WidthOfBlock(fmt);
		uint32_t rowCount = image->GetRowCount(mipMapLevel);
		uint8_t const* src = (uint8_t const*)TinyKtx_ImageRawData(ctx, mipMapLevel);

		uint32_t srcStride = TinyKtx_UnpackedRowStride(ctx, mipMapLevel);
		if( !TinyKtx_IsMipMapLevelUnpacked(ctx, mipMapLevel) )
			srcStride = (TinyImageFormat_BitSizeOfBlock(fmt) / 8) * blocksPerRow;

		uint32_t const dstStride = image->GetBytesPerRow(mipMapLevel);
		SE_ASSERT(srcStride <= dstStride);

		for( uint32_t ww = 0u; ww < s; ++ww )
		{
			uint8_t *dst = image->GetPixels(mipMapLevel, ww);
			for( uint32_t zz = 0; zz < d; ++zz )
			{
				for( uint32_t yy = 0; yy < rowCount; ++yy )
				{
					memcpy(dst, src, srcStride);
					src += srcStride;
					dst += dstStride;
				}
			}
		}
		w = max(w >> 1u, 1u);
		h = max(h >> 1u, 1u);
		d = max(d >> 1u, 1u);
	}

	TinyKtx_DestroyContext(ctx);
	return IMAGE_LOADING_RESULT_SUCCESS;
}
#endif
//-----------------------------------------------------------------------------
#ifndef IMAGE_DISABLE_GOOGLE_BASIS
ImageLoadingResult loadBASISFromStream(Image *image, FileStream *stream, MemoryAllocationFunc allocator, void *userData)
{
	if( stream == nullptr || fsGetStreamFileSize(stream) <= 0 )
		return IMAGE_LOADING_RESULT_DECODING_FAILED;

	basist::etc1_global_selector_codebook sel_codebook(basist::g_global_selector_cb_size, basist::g_global_selector_cb);

	void *basisData = fsGetStreamBufferIfPresent(stream);
	size_t memSize = (size_t)fsGetStreamFileSize(stream);

	if( !basisData )
	{
		basisData = conf_malloc(memSize);
		fsReadFromStream(stream, basisData, memSize);
	}

	basist::basisu_transcoder decoder(&sel_codebook);

	basist::basisu_file_info fileinfo;
	if( !decoder.get_file_info(basisData, (uint32_t)memSize, fileinfo) )
	{
		LOGF(LogLevel::eERROR, "Failed retrieving Basis file information!");
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	SE_ASSERT(fileinfo.m_total_images == fileinfo.m_image_mipmap_levels.size());
	SE_ASSERT(fileinfo.m_total_images == decoder.get_total_images(basisData, (uint32_t)memSize));

	basist::basisu_image_info imageinfo;
	decoder.get_image_info(basisData, (uint32_t)memSize, imageinfo, 0);

	uint32_t width = imageinfo.m_width;
	uint32_t height = imageinfo.m_height;
	uint32_t depth = 1;
	uint32_t mipMapCount = max(1U, fileinfo.m_image_mipmap_levels[0]);
	uint32_t arrayCount = fileinfo.m_total_images;

	TinyImageFormat imageFormat = TinyImageFormat_UNDEFINED;

	bool isNormalMap;

	if( fileinfo.m_userdata0 == 1 )
		isNormalMap = true;
	else
		isNormalMap = false;

	basist::transcoder_texture_format basisTextureFormat;

	if( !isNormalMap )
	{
		if( !imageinfo.m_alpha_flag )
		{
			imageFormat = TinyImageFormat_DXBC1_RGBA_UNORM;
			basisTextureFormat = basist::transcoder_texture_format::cTFBC1;
		}
		else
		{
			imageFormat = TinyImageFormat_DXBC3_UNORM;
			basisTextureFormat = basist::transcoder_texture_format::cTFBC3;
		}
	}
	else
	{
		imageFormat = TinyImageFormat_DXBC5_UNORM;
		basisTextureFormat = basist::transcoder_texture_format::cTFBC5;
	}

	image->RedefineDimensions(imageFormat, width, height, depth, mipMapCount, arrayCount);

	size_t size = image->GetSizeInBytes();

	if( allocator )
		image->SetPixels((unsigned char*)allocator(image, size, image->GetSubtextureAlignment(), userData));
	else
		image->SetPixels((unsigned char*)conf_malloc(sizeof(unsigned char) * size), true);

	if( !image->GetPixels() )
		return IMAGE_LOADING_RESULT_ALLOCATION_FAILED;

	decoder.start_transcoding(basisData, (uint32_t)memSize);

	for( uint32_t image_index = 0; image_index < fileinfo.m_total_images; image_index++ )
	{
		for( uint32_t level_index = 0; level_index < fileinfo.m_image_mipmap_levels[image_index]; level_index++ )
		{
			uint32_t rowPitchInBlocks = image->GetBytesPerRow(level_index) / (TinyImageFormat_BitSizeOfBlock(image->GetFormat()) / 8);
			SE_ASSERT(image->GetBytesPerRow(level_index) % (TinyImageFormat_BitSizeOfBlock(image->GetFormat()) / 8) == 0);

			basist::basisu_image_level_info level_info;

			if( !decoder.get_image_level_info(basisData, (uint32_t)memSize, level_info, image_index, level_index) )
			{
				LOGF(LogLevel::eERROR, "Failed retrieving image level information (%u %u)!\n", image_index, level_index);
				if( !fsGetStreamBufferIfPresent(stream) )
				{
					conf_free(basisData);
				}
				return IMAGE_LOADING_RESULT_DECODING_FAILED;
			}

			if( basisTextureFormat == basist::transcoder_texture_format::cTFPVRTC1_4_RGB )
			{
				if( !isPowerOf2(level_info.m_width) || !isPowerOf2(level_info.m_height) )
				{
					LOGF(LogLevel::eWARNING, "Warning: Will not transcode image %u level %u res %ux%u to PVRTC1 (one or more dimension is not a power of 2)\n", image_index, level_index, level_info.m_width, level_info.m_height);

					// Can't transcode this image level to PVRTC because it's not a pow2 (we're going to support transcoding non-pow2 to the next larger pow2 soon)
					continue;
				}
			}

			if( !decoder.transcode_image_level(basisData, (uint32_t)memSize, image_index, level_index, image->GetPixels(level_index, image_index), (uint32_t)(rowPitchInBlocks * imageinfo.m_num_blocks_y), basisTextureFormat, 0, rowPitchInBlocks) )
			{
				LOGF(LogLevel::eERROR, "Failed transcoding image level (%u %u)!\n", image_index, level_index);

				if( !fsGetStreamBufferIfPresent(stream) )
				{
					conf_free(basisData);
				}
				return IMAGE_LOADING_RESULT_DECODING_FAILED;
			}
		}
	}

	if( !fsGetStreamBufferIfPresent(stream) )
	{
		conf_free(basisData);
	}

	return IMAGE_LOADING_RESULT_SUCCESS;
}
#endif
//-----------------------------------------------------------------------------
ImageLoadingResult loadSVTFromStream(Image *image, FileStream *stream, MemoryAllocationFunc allocator, void *userData)
{
	if( stream == nullptr || fsGetStreamFileSize(stream) == 0 )
		return IMAGE_LOADING_RESULT_DECODING_FAILED;

	uint32_t width = fsReadFromStreamUInt32(stream);
	uint32_t height = fsReadFromStreamUInt32(stream);
	uint32_t mipMapCount = fsReadFromStreamUInt32(stream);
	/* uint32_t pageSize = */ fsReadFromStreamUInt32(stream);
	uint32_t numberOfComponents = fsReadFromStreamUInt32(stream);

	if( numberOfComponents == 4 )
		image->RedefineDimensions(TinyImageFormat_R8G8B8A8_UNORM, width, height, 1, mipMapCount, 1);
	else
		image->RedefineDimensions(TinyImageFormat_R8G8B8A8_UNORM, width, height, 1, mipMapCount, 1);

	size_t size = image->GetSizeInBytes();

	if( allocator )
		image->SetPixels((unsigned char*)allocator(image, size, image->GetSubtextureAlignment(), userData));
	else
		image->SetPixels((unsigned char*)conf_malloc(size), true);

	if( !image->GetPixels() )
		return IMAGE_LOADING_RESULT_ALLOCATION_FAILED;

	fsReadFromStream(stream, image->GetPixels(), size);

	return IMAGE_LOADING_RESULT_SUCCESS;
}
//-----------------------------------------------------------------------------
void Image::init()
{
#ifndef IMAGE_DISABLE_STB
	gImageLoaders[gImageLoaderCount++] = { "png", loadSTBFromStream };
	gImageLoaders[gImageLoaderCount++] = { "jpg", loadSTBFromStream };
#endif
	gImageLoaders[gImageLoaderCount++] = { "dds", loadDDSFromStream };
#ifndef IMAGE_DISABLE_KTX
	gImageLoaders[gImageLoaderCount++] = { "ktx", loadKTXFromStream };
#endif
#ifndef IMAGE_DISABLE_GOOGLE_BASIS
	gImageLoaders[gImageLoaderCount++] = { "basis", loadBASISFromStream };
#endif
	gImageLoaders[gImageLoaderCount++] = { "svt", loadSVTFromStream };
}
//-----------------------------------------------------------------------------
void Image::exit()
{
}
//-----------------------------------------------------------------------------
unsigned char* Image::create(const TinyImageFormat fmt, const int w, const int h, const int d, const int mipMapCount, const int arraySize, const unsigned char *rawData, const int rowAlignment, const int subtextureAlignment)
{
	m_format = fmt;
	m_width = w;
	m_height = h;
	m_depth = d;
	m_mipMapCount = mipMapCount;
	m_arrayCount = arraySize;
	m_ownsMemory = false;
	m_mipsAfterSlices = false;
	m_rowAlignment = rowAlignment;
	m_subtextureAlignment = subtextureAlignment;

	p_data = (uint8_t*)rawData;
	m_loadFilePath = nullptr;

	return p_data;
}
//-----------------------------------------------------------------------------
void Image::destroy()
{
	if( m_loadFilePath )
		m_loadFilePath = nullptr;

	if( p_data && m_ownsMemory )
	{
		conf_free(p_data);
		p_data = nullptr;
	}
}
//-----------------------------------------------------------------------------
void Image::clear()
{
	destroy();

	m_width = 0;
	m_height = 0;
	m_depth = 0;
	m_mipMapCount = 0;
	m_arrayCount = 0;
	m_format = TinyImageFormat_UNDEFINED;
	m_mipsAfterSlices = false;
	m_rowAlignment = 1;
}
//-----------------------------------------------------------------------------
ImageLoadingResult Image::loadFromFile(const Path *filePath, MemoryAllocationFunc allocator, void *userData, uint32_t rowAlignment, uint32_t subtextureAlignment)
{
	// clear current image
	clear();
	m_rowAlignment = rowAlignment;
	m_subtextureAlignment = subtextureAlignment;

	PathComponent extensionComponent = fsGetPathExtension(filePath);
	uint32_t loaderIndex = -1;

	if( extensionComponent.length != 0 ) // the string's not empty
	{
		for( int i = 0; i < (int)gImageLoaderCount; i++ )
		{
			if( stricmp(extensionComponent.buffer, gImageLoaders[i].Extension) == 0 )
			{
				loaderIndex = i;
				break;
			}
		}

		if( loaderIndex == -1 )
		{
			extensionComponent.buffer = nullptr;
			extensionComponent.length = 0;
		}
	}

	const char *extension;

	PathHandle loadFilePath = nullptr;
	// For loading basis file, it should have its extension
	if( extensionComponent.length == 0 )
	{
#if SE_PLATFORM_ANDROID
		extension = "ktx";
#else
		extension = "dds";
#endif

		loadFilePath = fsAppendPathExtension(filePath, extension);
	}
	else
	{
		extension = extensionComponent.buffer;
		loadFilePath = fsCopyPath(filePath);
	}

	FileStream *fh = fsOpenFile(loadFilePath, FM_READ_BINARY);
	FileStream *mem = nullptr;
	void *zipBuffer = nullptr;
	if( FSK_ZIP == fsGetFileSystemKind(fsGetPathFileSystem(loadFilePath)) )
	{
		ssize_t size = fsGetStreamFileSize(fh);
		zipBuffer = conf_malloc(size);
		mem = fsOpenReadOnlyMemory(zipBuffer, size);
		fsReadFromStream(fh, zipBuffer, size);
		fsCloseStream(fh);
		fh = mem;
	}

	if( !fh )
	{
		LOGF(LogLevel::eERROR, "\"%s\": Image file not found.", fsGetPathAsNativeString(loadFilePath));
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	// load file into memory

	ssize_t length = fsGetStreamFileSize(fh);
	if( length <= 0 )
	{
		LOGF(LogLevel::eERROR, "\"%s\": Image is an empty file.", fsGetPathAsNativeString(loadFilePath));
		fsCloseStream(fh);
		return IMAGE_LOADING_RESULT_DECODING_FAILED;
	}

	m_loadFilePath = loadFilePath;

	// try loading the format
	ImageLoadingResult result = IMAGE_LOADING_RESULT_DECODING_FAILED;
	bool support = false;
	for( int i = 0; i < (int)gImageLoaderCount; i++ )
	{
		if( stricmp(extension, gImageLoaders[i].Extension) == 0 )
		{
			support = true;
			result = gImageLoaders[i].Loader(this, fh, allocator, userData);
			if( result == IMAGE_LOADING_RESULT_SUCCESS || result == IMAGE_LOADING_RESULT_ALLOCATION_FAILED )
			{
				fsCloseStream(fh);
				if( zipBuffer )
					conf_free(zipBuffer);
				return result;
			}
			fsSeekStream(fh, SBO_START_OF_FILE, 0);
		}
	}
	if( !support )
	{
		LOGF(LogLevel::eERROR, "Can't load this file format for image:  %s", fsGetPathAsNativeString(loadFilePath));
	}

	fsCloseStream(fh);
	if( zipBuffer )
		conf_free(zipBuffer);

	return result;
}
//-----------------------------------------------------------------------------
ImageLoadingResult Image::loadFromStream(FileStream *stream, char const *extension, MemoryAllocationFunc allocator, void *userData, uint32_t rowAlignment, uint32_t subtextureAlignment)
{
	m_rowAlignment = rowAlignment;
	m_subtextureAlignment = subtextureAlignment;

	// try loading the format
	ImageLoadingResult result = IMAGE_LOADING_RESULT_DECODING_FAILED;
	for( uint32_t i = 0; i < gImageLoaderCount; ++i )
	{
		ImageLoaderDefinition const &def = gImageLoaders[i];
		if( extension && stricmp(extension, def.Extension) == 0 )
		{
			result = def.Loader(this, stream, allocator, userData);
			if( result == IMAGE_LOADING_RESULT_SUCCESS || result == IMAGE_LOADING_RESULT_ALLOCATION_FAILED )
			{
				return result;
			}

		}
	}
	return result;
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================