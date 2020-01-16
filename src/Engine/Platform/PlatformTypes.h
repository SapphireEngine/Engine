#pragma once

#if SE_PLATFORM_WINDOWS
#	if SE_ARCH_64BIT
typedef unsigned __int64 handle;
#	else
typedef unsigned __int32 handle;
#	endif
#elif SE_PLATFORM_LINUX
#	if SE_ARCH_64BIT
typedef uint64_t handle;
#	else
typedef uint32_t handle;
#	endif
#endif