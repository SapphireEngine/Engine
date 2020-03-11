#pragma once

//-----------------------------------------------------------------------------
// Base Engine Header
#include "Base/BaseHeader.h"

SE_PRAGMA_WARNING_PUSH
SE_PRAGMA_WARNING_DISABLE_MSVC(4324)

//-----------------------------------------------------------------------------
// STL Header
#include <cstdio>
#include <ctime>

#if SE_EASTL_ENABLED
#	include <EASTL/IMemory.h>
#	include <EASTL/internal/move_help.h>
#	include <EASTL/deque.h>
#	include <EASTL/string.h>
#	include <EASTL/vector.h>
#else
#	include <deque>
#	include <string>
#	include <vector>
#endif

//-----------------------------------------------------------------------------
// 3rdparty Header
#include "Base/STL.h"
#include "Platform/Base/PlatformHeader.h"

//-----------------------------------------------------------------------------
// Other Engine Header

SE_PRAGMA_WARNING_POP