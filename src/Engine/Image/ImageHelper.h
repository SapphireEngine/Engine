#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

static inline uint32_t CalculateMipMapLevels(uint32_t width, uint32_t height)
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

static inline uint32_t ImageGetMipMappedSize(uint32_t w, uint32_t h, uint32_t d, uint32_t mipMapLevels, TinyImageFormat srcFormat)
{
	// PVR formats get special case
	if ( srcFormat == TinyImageFormat_PVRTC1_2BPP_UNORM || srcFormat == TinyImageFormat_PVRTC1_2BPP_SRGB ||
		 srcFormat == TinyImageFormat_PVRTC1_4BPP_UNORM || srcFormat == TinyImageFormat_PVRTC1_4BPP_SRGB )
	{
		uint32_t totalSize = 0;
		uint32_t sizeX = w;
		uint32_t sizeY = h;
		uint32_t sizeD = d;
		int level = mipMapLevels;

		uint32_t minWidth  = 8;
		uint32_t minHeight = 8;
		uint32_t minDepth  = 1;
		int bpp = 4;

		if ( srcFormat == TinyImageFormat_PVRTC1_2BPP_UNORM || srcFormat == TinyImageFormat_PVRTC1_2BPP_SRGB )
		{
			minWidth = 16;
			minHeight = 8;
			bpp = 2;
		}

		while ( level > 0 )
		{
			// If pixel format is compressed, the dimensions need to be padded.
			const uint32_t paddedWidth =  sizeX + ((-1 * sizeX) % minWidth);
			const uint32_t paddedHeight = sizeY + ((-1 * sizeY) % minHeight);
			const uint32_t paddedDepth =  sizeD + ((-1 * sizeD) % minDepth);

			const uint32_t mipSize = paddedWidth * paddedHeight * paddedDepth * bpp / 8;

			totalSize += mipSize;

			const unsigned int MinimumSize = 1;
			sizeX = max(sizeX / 2, MinimumSize);
			sizeY = max(sizeY / 2, MinimumSize);
			sizeD = max(sizeD / 2, MinimumSize);
			level--;
		}

		return totalSize;
	}

	uint32_t size = 0;
	while ( mipMapLevels )
	{
		uint32_t bx = TinyImageFormat_WidthOfBlock(srcFormat);
		uint32_t by = TinyImageFormat_HeightOfBlock(srcFormat);
		uint32_t bz = TinyImageFormat_DepthOfBlock(srcFormat);
		size += ((w + bx - 1) / bx) * ((h + by - 1) / by) * ((d + bz - 1) / bz);

		w >>= 1;
		h >>= 1;
		d >>= 1;
		if ( w + h + d == 0 )
			break;
		if ( w == 0 ) w = 1;
		if ( h == 0 ) h = 1;
		if ( d == 0 ) d = 1;

		mipMapLevels--;
	}

	size *= TinyImageFormat_BitSizeOfBlock(srcFormat) / 8;

	return size;
}

SE_NAMESPACE_END
//=============================================================================