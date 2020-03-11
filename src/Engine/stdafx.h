#pragma once

//-----------------------------------------------------------------------------
// Base Engine Header
#include "Base/BaseHeader.h"

SE_PRAGMA_WARNING_PUSH
SE_PRAGMA_WARNING_DISABLE_MSVC(4324)

//-----------------------------------------------------------------------------
// STL Header
#include <cstdio>

#if SE_EASTL_ENABLED
#	include <EASTL/internal/move_help.h>
#else
#	include <string>
#	include <string_view>
#	include <vector>
#	include <unordered_map>
#endif


//-----------------------------------------------------------------------------
// 3rdparty Header
#include "Base/STL.h"
#include "Platform/Base/PlatformHeader.h"

//-----------------------------------------------------------------------------
// Other Engine Header

SE_PRAGMA_WARNING_POP