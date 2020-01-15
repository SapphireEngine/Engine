#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

enum class RenderingBackend
{
	Direct3D11,
	Direct3D12,
	OpenGL,
	Vulkan
};

struct RenderConfig
{
	RenderingBackend render = RenderingBackend::Vulkan;
};

// Calculate the mipmap size at the given mipmap index
inline void GetMipmapSize(uint32_t mipmapIndex, uint32_t &width)
{
	if ( 0u != mipmapIndex )
	{
		width = static_cast<uint32_t>(static_cast<float>(width) / std::exp2f(static_cast<float>(mipmapIndex)));
		if ( 0u == width )
		{
			width = 1u;
		}
	}
}

// Calculate the mipmap size at the given mipmap index
inline void GetMipmapSize(uint32_t mipmapIndex, uint32_t &width, uint32_t &height)
{
	GetMipmapSize(mipmapIndex, width);
	GetMipmapSize(mipmapIndex, height);
}

inline void UpdateWidthHeight(uint32_t mipmapIndex, uint32_t textureWidth, uint32_t textureHeight, uint32_t &width, uint32_t &height)
{
	GetMipmapSize(mipmapIndex, textureWidth, textureHeight);
	if ( width > textureWidth )
		width = textureWidth;
	if ( height > textureHeight )
		height = textureHeight;
}

SE_NAMESPACE_END
//=============================================================================