#include "stdafx.h"
#include "VKSwapChain.h"
#include "VKContext.h"
#include "Platform/Window/Window.h"
#include "VKRenderer.h"

#if SE_VULKAN

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
[[nodiscard]] VkSurfaceKHR createPresentationSurface(const VkAllocationCallbacks *vkAllocationCallbacks, VkInstance vkInstance, VkPhysicalDevice vkPhysicalDevice, uint32_t graphicsQueueFamilyIndex, window::WindowHandle windoInfo)
{
	VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

#ifdef VK_USE_PLATFORM_WIN32_KHR
	const VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR =
	{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, // sType (VkStructureType)
		nullptr, // pNext (const void*)
		0, // flags (VkWin32SurfaceCreateFlagsKHR)
		reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(reinterpret_cast<HWND>(windoInfo.nativeWindowHandle), GWLP_HINSTANCE)), // hinstance (HINSTANCE)
		reinterpret_cast<HWND>(windoInfo.nativeWindowHandle) // hwnd (HWND)
	};
	if ( vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, vkAllocationCallbacks, &vkSurfaceKHR) != VK_SUCCESS )
	{
		// TODO(co) Can we ensure "vkSurfaceKHR" doesn't get touched by "vkCreateWin32SurfaceKHR()" in case of failure?
		vkSurfaceKHR = VK_NULL_HANDLE;
	}
#elif defined VK_USE_PLATFORM_ANDROID_KHR
	const VkAndroidSurfaceCreateInfoKHR vkAndroidSurfaceCreateInfoKHR =
	{
		VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,				// sType (VkStructureType)
		nullptr,														// pNext (const void*)
		0,																// flags (VkAndroidSurfaceCreateFlagsKHR)
		reinterpret_cast<ANativeWindow*>(windoInfo.nativeWindowHandle)	// window (ANativeWindow*)
	};
	if ( vkCreateAndroidSurfaceKHR(vkInstance, &vkAndroidSurfaceCreateInfoKHR, vkAllocationCallbacks, &vkSurfaceKHR) != VK_SUCCESS )
	{
		// TODO(co) Can we ensure "vkSurfaceKHR" doesn't get touched by "vkCreateAndroidSurfaceKHR()" in case of failure?
		vkSurfaceKHR = VK_NULL_HANDLE;
	}
#elif defined VK_USE_PLATFORM_XLIB_KHR || defined VK_USE_PLATFORM_WAYLAND_KHR
	SE_ASSERT(context, context.getType() == Context::ContextType::X11 || context.getType() == Context::ContextType::WAYLAND, "Invalid Vulkan context type")

		// If the given RHI context is an X11 context use the display connection object provided by the context
		if ( context.getType() == Context::ContextType::X11 )
		{
			const X11Context& x11Context = static_cast<const X11Context&>(context);
			const VkXlibSurfaceCreateInfoKHR vkXlibSurfaceCreateInfoKHR =
			{
				VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,	// sType (VkStructureType)
				nullptr,										// pNext (const void*)
				0,												// flags (VkXlibSurfaceCreateFlagsKHR)
				x11Context.getDisplay(),						// dpy (Display*)
				windoInfo.nativeWindowHandle					// window (Window)
			};
			if ( vkCreateXlibSurfaceKHR(vkInstance, &vkXlibSurfaceCreateInfoKHR, vkAllocationCallbacks, &vkSurfaceKHR) != VK_SUCCESS )
			{
				// TODO(co) Can we ensure "vkSurfaceKHR" doesn't get touched by "vkCreateXlibSurfaceKHR()" in case of failure?
				vkSurfaceKHR = VK_NULL_HANDLE;
			}
		}
		else if ( context.getType() == Context::ContextType::WAYLAND )
		{
			const WaylandContext& waylandContext = static_cast<const WaylandContext&>(context);
			const VkWaylandSurfaceCreateInfoKHR vkWaylandSurfaceCreateInfoKHR =
			{
				VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,	// sType (VkStructureType)
				nullptr,											// pNext (const void*)
				0,													// flags (VkWaylandSurfaceCreateInfoKHR)
				waylandContext.getDisplay(),						// display (wl_display*)
				windoInfo.waylandSurface							// surface (wl_surface*)
			};
			if ( vkCreateWaylandSurfaceKHR(vkInstance, &vkWaylandSurfaceCreateInfoKHR, vkAllocationCallbacks, &vkSurfaceKHR) != VK_SUCCESS )
			{
				// TODO(co) Can we ensure "vkSurfaceKHR" doesn't get touched by "vkCreateWaylandSurfaceKHR()" in case of failure?
				vkSurfaceKHR = VK_NULL_HANDLE;
			}
		}
#elif defined VK_USE_PLATFORM_XCB_KHR
#error "TODO(co) Complete implementation"
	const VkXcbSurfaceCreateInfoKHR vkXcbSurfaceCreateInfoKHR =
	{
		VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,	// sType (VkStructureType)
		nullptr,										// pNext (const void*)
		0,												// flags (VkXcbSurfaceCreateFlagsKHR)
		TODO(co)										// connection (xcb_connection_t*)
		TODO(co)										// window (xcb_window_t)
	};
	if ( vkCreateXcbSurfaceKHR(vkInstance, &vkXcbSurfaceCreateInfoKHR, vkAllocationCallbacks, &vkSurfaceKHR) != VK_SUCCESS )
	{
		// TODO(co) Can we ensure "vkSurfaceKHR" doesn't get touched by "vkCreateXcbSurfaceKHR()" in case of failure?
		vkSurfaceKHR = VK_NULL_HANDLE;
	}
#else
#error "Unsupported platform"
#endif

	{ // Sanity check: Does the physical Vulkan device support the Vulkan presentation surface?
	  // TODO(co) Inside our RHI implementation the swap chain is physical device independent, which is a nice thing usability wise.
	  //          On the other hand, the sanity check here can only detect issues but it would be better to not get into such issues in the first place.
		VkBool32 queuePresentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, graphicsQueueFamilyIndex, vkSurfaceKHR, &queuePresentSupport);
		if ( VK_FALSE == queuePresentSupport )
		{
			SE_LOG(CRITICAL, "The created Vulkan presentation surface has no queue Present support")
		}
	}

	return vkSurfaceKHR;
}
//-----------------------------------------------------------------------------
[[nodiscard]] VkSurfaceFormatKHR getSwapChainFormat(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurfaceKHR)
{
	uint32_t surfaceFormatCount = 0;
	if ( (vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurfaceKHR, &surfaceFormatCount, nullptr) != VK_SUCCESS) || (0 == surfaceFormatCount) )
	{
		SE_LOG(CRITICAL, "Failed to get physical Vulkan device surface formats")
		return { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	if ( vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurfaceKHR, &surfaceFormatCount, surfaceFormats.data()) != VK_SUCCESS )
	{
		SE_LOG(CRITICAL, "Failed to get physical Vulkan device surface formats")
		return { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
	}

	// If the list contains only one entry with undefined format it means that there are no preferred surface formats and any can be chosen
	if ( (surfaceFormats.size() == 1) && (VK_FORMAT_UNDEFINED == surfaceFormats[0].format) )
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	}

	// Check if list contains most widely used R8 G8 B8 A8 format with nonlinear color space
	// -> Not all implementations support RGBA8, some only support BGRA8 formats (e.g. xlib surface under Linux with RADV), so check for both
	for ( const VkSurfaceFormatKHR& surfaceFormat : surfaceFormats )
	{
		if ( VK_FORMAT_R8G8B8A8_UNORM == surfaceFormat.format || VK_FORMAT_B8G8R8A8_UNORM == surfaceFormat.format )
		{
			return surfaceFormat;
		}
	}

	// Return the first format from the list
	return surfaceFormats[0];
}
//-----------------------------------------------------------------------------
[[nodiscard]] VkFormat findSupportedVkFormat(VkPhysicalDevice vkPhysicalDevice, const std::vector<VkFormat> &vkFormatCandidates, VkImageTiling vkImageTiling, VkFormatFeatureFlags vkFormatFeatureFlags)
{
	for ( VkFormat vkFormat : vkFormatCandidates )
	{
		VkFormatProperties vkFormatProperties;
		vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice, vkFormat, &vkFormatProperties);
		if ( VK_IMAGE_TILING_LINEAR == vkImageTiling && (vkFormatProperties.linearTilingFeatures & vkFormatFeatureFlags) == vkFormatFeatureFlags )
		{
			return vkFormat;
		}
		else if ( VK_IMAGE_TILING_OPTIMAL == vkImageTiling && (vkFormatProperties.optimalTilingFeatures & vkFormatFeatureFlags) == vkFormatFeatureFlags )
		{
			return vkFormat;
		}
	}

	// Failed to find supported Vulkan depth format
	return VK_FORMAT_UNDEFINED;
}
//-----------------------------------------------------------------------------
VkFormat SwapChain::FindColorVkFormat(VkInstance vkInstance, const VulkanContext &vulkanContext)
{
	const VkPhysicalDevice vkPhysicalDevice = vulkanContext.GetVkPhysicalDevice();
	const VkSurfaceKHR vkSurfaceKHR = createPresentationSurface(vulkanContext.GetRender().GetVkAllocationCallbacks(), vkInstance, vkPhysicalDevice, vulkanContext.GetGraphicsQueueFamilyIndex(), window::WindowHandle{ (handle)GetSubsystem<window::Window>().hwnd, nullptr });

	const VkSurfaceFormatKHR desiredVkSurfaceFormatKHR = getSwapChainFormat(vkPhysicalDevice, vkSurfaceKHR);
	vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, vulkanContext.GetRender().GetVkAllocationCallbacks());
	return desiredVkSurfaceFormatKHR.format;
}
//-----------------------------------------------------------------------------
VkFormat SwapChain::FindDepthVkFormat(VkPhysicalDevice vkPhysicalDevice)
{
	return findSupportedVkFormat(vkPhysicalDevice, { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================

#endif