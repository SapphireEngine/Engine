#pragma once

#if SE_VULKAN

#	if SE_PLATFORM_WINDOWS
#		define VK_USE_PLATFORM_WIN32_KHR
#	elif SE_PLATFORM_LINUX
#		define VK_USE_PLATFORM_XLIB_KHR
#		define VK_USE_PLATFORM_WAYLAND_KHR
#	elif SE_PLATFORM_ANDROID
#		define VK_USE_PLATFORM_ANDROID_KHR
#	endif

#	if 0
#		include <vulkan/vulkan.hpp>
#	else
#		define VK_NO_PROTOTYPES
#		include <vulkan/vulkan.h>
#		include "RenderingBackend/Vulkan/VKFuncPrototypes.h"
#	endif
#endif

//=============================================================================
SE_NAMESPACE_BEGIN

#if SE_VULKAN

#endif

SE_NAMESPACE_END
//=============================================================================