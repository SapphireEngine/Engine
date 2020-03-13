#pragma once

// Enable EASTL
#define SE_EASTL_ENABLED        1

// Enable Profiling
#define SE_PROFILING_ENABLED    1

// Enable Statistics
#define SE_STATISTICS           1

// Enable GAPI
#define SE_ENABLE_OPENGL        1
#define SE_ENABLE_DIRECT3D11    1
#define SE_ENABLE_DIRECT3D12    1
#define SE_ENABLE_VULKAN        1

// Specific GAPI config
#define SE_OPENGL_VERSION_MAJOR 4
#define SE_OPENGL_VERSION_MINOR 6



// TODO:
//#define VULKAN 1
//#define DIRECT3D11 1
#define DIRECT3D12 1
#define _WINDOWS
#define USE_MEMORY_TRACKING
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE