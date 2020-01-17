#ifdef VKHEADER_IMPL
#	define FNPTR(name) PFN_##name name;
#else
#	define FNPTR(name) extern PFN_##name name;
#endif

// Global Vulkan function pointers
FNPTR(vkGetInstanceProcAddr)
FNPTR(vkGetDeviceProcAddr)
FNPTR(vkEnumerateInstanceExtensionProperties)
FNPTR(vkEnumerateInstanceLayerProperties)
FNPTR(vkCreateInstance)

// Instance based Vulkan function pointers
FNPTR(vkDestroyInstance)
FNPTR(vkEnumeratePhysicalDevices)
FNPTR(vkEnumerateDeviceLayerProperties)
FNPTR(vkEnumerateDeviceExtensionProperties)
FNPTR(vkGetPhysicalDeviceQueueFamilyProperties)
FNPTR(vkGetPhysicalDeviceFeatures)
FNPTR(vkGetPhysicalDeviceFormatProperties)
FNPTR(vkGetPhysicalDeviceMemoryProperties)
FNPTR(vkGetPhysicalDeviceProperties)
FNPTR(vkCreateDevice)
// "VK_EXT_debug_report"-extension
FNPTR(vkCreateDebugReportCallbackEXT)
FNPTR(vkDestroyDebugReportCallbackEXT)
// "VK_KHR_surface"-extension
FNPTR(vkDestroySurfaceKHR)
FNPTR(vkGetPhysicalDeviceSurfaceSupportKHR)
FNPTR(vkGetPhysicalDeviceSurfaceFormatsKHR)
FNPTR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
FNPTR(vkGetPhysicalDeviceSurfacePresentModesKHR)
#ifdef VK_USE_PLATFORM_WIN32_KHR
// "VK_KHR_win32_surface"-extension
FNPTR(vkCreateWin32SurfaceKHR)
#elif defined VK_USE_PLATFORM_ANDROID_KHR
// "VK_KHR_android_surface"-extension
FNPTR(vkCreateAndroidSurfaceKHR)
#elif defined VK_USE_PLATFORM_XLIB_KHR || defined VK_USE_PLATFORM_WAYLAND_KHR
#if defined VK_USE_PLATFORM_XLIB_KHR
// "VK_KHR_xlib_surface"-extension
FNPTR(vkCreateXlibSurfaceKHR)
#endif
#if defined VK_USE_PLATFORM_WAYLAND_KHR
// "VK_KHR_wayland_surface"-extension
FNPTR(vkCreateWaylandSurfaceKHR)
#endif
#elif defined VK_USE_PLATFORM_XCB_KHR
// "VK_KHR_xcb_surface"-extension
FNPTR(vkCreateXcbSurfaceKHR)
#else
#error "Unsupported platform"
#endif

// Device based Vulkan function pointers
FNPTR(vkDestroyDevice)
FNPTR(vkCreateShaderModule)
FNPTR(vkDestroyShaderModule)
FNPTR(vkCreateBuffer)
FNPTR(vkDestroyBuffer)
FNPTR(vkMapMemory)
FNPTR(vkUnmapMemory)
FNPTR(vkCreateBufferView)
FNPTR(vkDestroyBufferView)
FNPTR(vkAllocateMemory)
FNPTR(vkFreeMemory)
FNPTR(vkGetBufferMemoryRequirements)
FNPTR(vkBindBufferMemory)
FNPTR(vkCreateRenderPass)
FNPTR(vkDestroyRenderPass)
FNPTR(vkCreateImage)
FNPTR(vkDestroyImage)
FNPTR(vkGetImageSubresourceLayout)
FNPTR(vkGetImageMemoryRequirements)
FNPTR(vkBindImageMemory)
FNPTR(vkCreateImageView)
FNPTR(vkDestroyImageView)
FNPTR(vkCreateSampler)
FNPTR(vkDestroySampler)
FNPTR(vkCreateSemaphore)
FNPTR(vkDestroySemaphore)
FNPTR(vkCreateFence)
FNPTR(vkDestroyFence)
FNPTR(vkWaitForFences)
FNPTR(vkCreateCommandPool)
FNPTR(vkDestroyCommandPool)
FNPTR(vkAllocateCommandBuffers)
FNPTR(vkFreeCommandBuffers)
FNPTR(vkBeginCommandBuffer)
FNPTR(vkEndCommandBuffer)
FNPTR(vkGetDeviceQueue)
FNPTR(vkQueueSubmit)
FNPTR(vkQueueWaitIdle)
FNPTR(vkDeviceWaitIdle)
FNPTR(vkCreateFramebuffer)
FNPTR(vkDestroyFramebuffer)
FNPTR(vkCreatePipelineCache)
FNPTR(vkDestroyPipelineCache)
FNPTR(vkCreatePipelineLayout)
FNPTR(vkDestroyPipelineLayout)
FNPTR(vkCreateGraphicsPipelines)
FNPTR(vkCreateComputePipelines)
FNPTR(vkDestroyPipeline)
FNPTR(vkCreateDescriptorPool)
FNPTR(vkDestroyDescriptorPool)
FNPTR(vkCreateDescriptorSetLayout)
FNPTR(vkDestroyDescriptorSetLayout)
FNPTR(vkAllocateDescriptorSets)
FNPTR(vkFreeDescriptorSets)
FNPTR(vkUpdateDescriptorSets)
FNPTR(vkCreateQueryPool)
FNPTR(vkDestroyQueryPool)
FNPTR(vkGetQueryPoolResults)
FNPTR(vkCmdBeginQuery)
FNPTR(vkCmdEndQuery)
FNPTR(vkCmdResetQueryPool)
FNPTR(vkCmdWriteTimestamp)
FNPTR(vkCmdCopyQueryPoolResults)
FNPTR(vkCmdPipelineBarrier)
FNPTR(vkCmdBeginRenderPass)
FNPTR(vkCmdEndRenderPass)
FNPTR(vkCmdExecuteCommands)
FNPTR(vkCmdCopyImage)
FNPTR(vkCmdBlitImage)
FNPTR(vkCmdCopyBufferToImage)
FNPTR(vkCmdClearAttachments)
FNPTR(vkCmdCopyBuffer)
FNPTR(vkCmdBindDescriptorSets)
FNPTR(vkCmdBindPipeline)
FNPTR(vkCmdSetViewport)
FNPTR(vkCmdSetScissor)
FNPTR(vkCmdSetLineWidth)
FNPTR(vkCmdSetDepthBias)
FNPTR(vkCmdPushConstants)
FNPTR(vkCmdBindIndexBuffer)
FNPTR(vkCmdBindVertexBuffers)
FNPTR(vkCmdDraw)
FNPTR(vkCmdDrawIndexed)
FNPTR(vkCmdDrawIndirect)
FNPTR(vkCmdDrawIndexedIndirect)
FNPTR(vkCmdDispatch)
FNPTR(vkCmdClearColorImage)
FNPTR(vkCmdClearDepthStencilImage)
// "VK_EXT_debug_marker"-extension
FNPTR(vkDebugMarkerSetObjectTagEXT)
FNPTR(vkDebugMarkerSetObjectNameEXT)
FNPTR(vkCmdDebugMarkerBeginEXT)
FNPTR(vkCmdDebugMarkerEndEXT)
FNPTR(vkCmdDebugMarkerInsertEXT)
// "VK_KHR_swapchain"-extension
FNPTR(vkCreateSwapchainKHR)
FNPTR(vkDestroySwapchainKHR)
FNPTR(vkGetSwapchainImagesKHR)
FNPTR(vkAcquireNextImageKHR)
FNPTR(vkQueuePresentKHR)

#undef FNPTR