#pragma once

#if SE_DIRECT3D12
#	include <d3d12.h>
#	include <D3Dcompiler.h>
#	include <dxgi1_4.h>
#	include "Platform/Window/Window.h"
#endif

//=============================================================================
SE_NAMESPACE_BEGIN

#if SE_DIRECT3D12

inline IDXGISwapChain1* CreateSwapchain(SE_NAMESPACE_WND::Window *window, IDXGIFactory4 *factory, ID3D12CommandQueue *queue, DXGI_SWAP_CHAIN_DESC1 *swapchainDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC *fullscreenDesc = nullptr, IDXGIOutput *output = nullptr)
{
	IDXGISwapChain1 *swapchain = nullptr;
	HRESULT hr = factory->CreateSwapChainForHwnd(queue, window->hwnd, swapchainDesc, fullscreenDesc, output, &swapchain);
	if ( !FAILED(hr) )
		return swapchain;

	return nullptr;
}

#endif

SE_NAMESPACE_END
//=============================================================================