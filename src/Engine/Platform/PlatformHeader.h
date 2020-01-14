#pragma once

#include "Platform/PlatformMacros.h"

#if SE_PLATFORM_WINDOWS
#	include "Platform/ConfigPlatformWin32.h"
#	include <Windows.h>
#	include <Shobjidl.h>
#	include <dwmapi.h>
#	include <windowsx.h>
#	include <direct.h>
#else
#	include <unistd.h>
#endif