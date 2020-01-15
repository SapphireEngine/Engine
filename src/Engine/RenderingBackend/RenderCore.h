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

SE_NAMESPACE_END
//=============================================================================