#pragma once

#if SE_PLATFORM_WINDOWS
#	include "Platform/Base/Win32PlatformDefinitions.h"
#	include <Windows.h>
#	include <timeapi.h>
#	include <io.h>
#	include <intrin.h>
#	include <WinBase.h>
#	include <shlobj.h>
#	include <commdlg.h>
#endif

#include "Platform/Base/PlatformMacros.h"