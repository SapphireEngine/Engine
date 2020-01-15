#pragma once

#if SE_DIRECT3D11
#	include <d3d11.h>
#	include <D3Dcompiler.h>
#	include <dxgi1_2.h>
#	include "Platform/Window/Window.h"
#endif

//=============================================================================
SE_NAMESPACE_BEGIN

#if SE_DIRECT3D11
inline IDXGISwapChain* CreateSwapchain(SE_NAMESPACE_WND::Window *window, IDXGIFactory *factory, ID3D11Device *device, DXGI_SWAP_CHAIN_DESC *swapchainDesc)
{
	SE_NAMESPACE_WND::WindowConfig desc = window->GetDesc();

	swapchainDesc->OutputWindow = window->hwnd;
	swapchainDesc->Windowed = !desc.fullscreen;

	IDXGISwapChain *swapchain = nullptr;
	HRESULT hr = factory->CreateSwapChain(device, swapchainDesc, &swapchain);
	if ( !FAILED(hr) )
		return swapchain;

	return nullptr;
}
#endif

SE_NAMESPACE_END
//=============================================================================