#pragma once

#if SE_PLATFORM_WINDOWS
#	include "Platform/ConfigPlatformWin32.h"
#	include <Windows.h>
#	include <Shobjidl.h>
#	include <dwmapi.h>
#	include <windowsx.h>
#	include <direct.h>
#elif SE_PLATFORM_LINUX
#	include <unistd.h>
#endif

#include "Platform/PlatformMacros.h"
#include "Platform/PlatformTypes.h"