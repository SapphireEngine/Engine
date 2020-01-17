#pragma once

#include "RenderingBackend/Resource/IRenderTarget.h"

//=============================================================================
SE_NAMESPACE_BEGIN

// Abstract render window interface which is used to implement some platform specific functionality regarding render window needed by the swap chain implementation
class IRenderWindow
{
public:
	virtual ~IRenderWindow() = default;

	virtual void GetWidthAndHeight(uint32_t &width, uint32_t &height) const = 0;
	virtual void Present() = 0;

protected:
	IRenderWindow() = default;
	explicit IRenderWindow(const IRenderWindow&) = delete;
	IRenderWindow& operator=(const IRenderWindow&) = delete;
};

class ISwapChain : public IRenderTarget
{
public:
	virtual ~ISwapChain() override
	{
#if SE_STATISTICS
		--GetRender().GetStatistics().currentNumberOfSwapChains;
#endif
	}

	// Return the native window handle
	[[nodiscard]] virtual handle GetNativeWindowHandle() const = 0;

	// Set vertical synchronization interval
	virtual void SetVerticalSynchronizationInterval(uint32_t synchronizationInterval) = 0;

	// Present the content of the current back buffer
	virtual void Present() = 0;

	// Call this method whenever the size of the native window was changed
	virtual void ResizeBuffers() = 0;

	// Return the current fullscreen state
	[[nodiscard]] virtual bool GetFullscreenState() const = 0;

	// Set the current fullscreen state
	virtual void SetFullscreenState(bool fullscreen) = 0;

	// Set an render window instance
	virtual void SetRenderWindow(IRenderWindow *renderWindow) = 0;


protected:
	explicit ISwapChain(IRenderPass &renderPass SE_DEBUG_NAME_PARAMETER_NO_DEFAULT) 
		: IRenderTarget(ResourceType::SWAP_CHAIN, renderPass SE_DEBUG_PASS_PARAMETER)
	{
#if SE_STATISTICS
		// Update the statistics
		++GetRender().GetStatistics().numberOfCreatedSwapChains;
		++GetRender().GetStatistics().currentNumberOfSwapChains;
#endif
	}

	explicit ISwapChain(const ISwapChain&) = delete;
	ISwapChain& operator=(const ISwapChain&) = delete;
};

typedef SmartRefCount<ISwapChain> ISwapChainPtr;

SE_NAMESPACE_END
//=============================================================================