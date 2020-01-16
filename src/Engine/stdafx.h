#pragma once

//-----------------------------------------------------------------------------
// Base Engine Header
#include "Base/BaseHeader.h"
#include "Platform/PlatformHeader.h"

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
#include <mutex>
#include <string>
#include <string_view>
#include <queue>
#include <vector>
#include <unordered_map>

//-----------------------------------------------------------------------------
// 3rdparty Header
#include <vectormath/vectormath.hpp>

//-----------------------------------------------------------------------------
// Other Engine Header
#include "RenderingBackend/CoreHeader/Vulkan.h"
#include "Core/Template/Ref.h"

SE_PRAGMA_WARNING_POP