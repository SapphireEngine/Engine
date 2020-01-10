#pragma once

//-----------------------------------------------------------------------------
// Base Engine Header
#include "Base/BaseHeader.h"
#include "Platform/PlatformHeader.h"

//-----------------------------------------------------------------------------
// STL Header
#include <type_traits>
#include <codecvt>
#include <locale>
#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include <queue>
#include <unordered_map>

//-----------------------------------------------------------------------------
// 3rdparty Header
#include <vectormath/vectormath.hpp>

//-----------------------------------------------------------------------------
// Other Engine Header
#include "RenderingBackend/Direct3D11.h"
#include "RenderingBackend/Direct3D12.h"
#include "RenderingBackend/OpenGL.h"
#include "RenderingBackend/Vulkan.h"