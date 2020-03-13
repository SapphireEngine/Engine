#include "stdafx.h"
#ifdef DIRECT3D12
#include "Renderer/Renderer.h"
#include "Direct3D12Hooks.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
static void enable_debug_layer_hook(Renderer* pRenderer)
{
#if defined(_DEBUG) || defined(PROFILE)
	pRenderer->pDXDebug->EnableDebugLayer();
	ID3D12Debug1* pDebug1 = NULL;
	if ( S_OK == pRenderer->pDXDebug->QueryInterface(IID_PPV_ARGS(&pDebug1)) )
	{
		pDebug1->SetEnableGPUBasedValidation(pRenderer->mEnableGpuBasedValidation);
		pDebug1->Release();
	}
#endif
}

static TinyImageFormat get_recommended_swapchain_format(bool hintHDR)
{
	return TinyImageFormat_B8G8R8A8_UNORM;
}

static uint32_t get_swap_chain_image_index(SwapChain* pSwapChain)
{
	return pSwapChain->pDxSwapChain->GetCurrentBackBufferIndex();
}

void initHooks()
{
	fnHookEnableDebugLayer = enable_debug_layer_hook;
	fnHookGetRecommendedSwapChainFormat = get_recommended_swapchain_format;
	fnHookGetSwapChainImageIndex = get_swap_chain_image_index;
}
SE_NAMESPACE_END
//=============================================================================
#endif