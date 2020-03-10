#pragma once

//=============================================================================
// Platform Macros
//=============================================================================
#if SE_COMPILER_MSVC
#	define SE_NO_VTABLE __declspec(novtable) // https://habr.com/ru/post/442340/
#	define SE_FORCEINLINE __forceinline
#elif SE_COMPILER_CLANG
#	define SE_NO_VTABLE
#	define SE_FORCEINLINE __attribute__((always_inline))
#elif SE_COMPILER_GNUC
#	define SE_NO_VTABLE
#	define SE_FORCEINLINE __attribute__((always_inline))
#endif

#if SE_PLATFORM_WINDOWS
#	define SE_DEBUG_BREAK __debugbreak()
#elif SE_PLATFORM_LINUX
#	define SE_DEBUG_BREAK __builtin_trap()
#endif