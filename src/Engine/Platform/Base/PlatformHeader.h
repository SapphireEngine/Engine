#pragma once

#if SE_PLATFORM_WINDOWS
#	include "Platform/Base/Win32PlatformDefinitions.h"
#	include <Windows.h>
#	include <Shobjidl.h>
#	include <dwmapi.h>
#	include <windowsx.h>
#	include <direct.h>
#elif SE_PLATFORM_LINUX
#	include <unistd.h>
#endif

#include "Platform/Base/PlatformMacros.h"
#include "Platform/Base/PlatformTypes.h"