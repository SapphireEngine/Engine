#pragma once

//-----------------------------------------------------------------------------
// Base Engine Header
#include "Base/BaseHeader.h"
#include "Platform/Base/PlatformHeader.h"

SE_PRAGMA_WARNING_PUSH
SE_PRAGMA_WARNING_DISABLE_MSVC(4324)

//-----------------------------------------------------------------------------
// STL Header
#include <type_traits>
#include <codecvt>
#include <locale>
#include <memory>
#include <functional>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <atomic>
#include <mutex>
#include <string>
#include <string_view>
#include <array>
#include <queue>
#include <vector>
#include <unordered_map>

//-----------------------------------------------------------------------------
// 3rdparty Header

//-----------------------------------------------------------------------------
// Other Engine Header
#include "RenderingBackend/CoreTypes/CoreTypes.h"

#include "RenderingBackend/CoreSystem/IRendererBackend.h"

SE_PRAGMA_WARNING_POP