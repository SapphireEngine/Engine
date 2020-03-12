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

// TODO: remove
#if SE_PLATFORM_WINDOWS
#	define FORGE_CALLCONV __cdecl
#else
#	define FORGE_CALLCONV
#endif

//For getting rid of unreferenced parameter warnings
#ifdef _MSC_VER    //If on Visual Studio
#define UNREF_PARAM(x) (x)
#elif defined(ORBIS)
#define UNREF_PARAM(x) ((void)(x))
#else
//Add more compilers and platforms as we need them
#define UNREF_PARAM(x)
#endif

#if   INTPTR_MAX == 0x7FFFFFFFFFFFFFFFLL
# define PTR_SIZE 8
#elif INTPTR_MAX == 0x7FFFFFFF
# define PTR_SIZE 4
#else
#error unsupported platform
#endif

#ifdef _MSC_VER
#define ALIGNAS(x) __declspec( align( x ) ) 
#else
#define ALIGNAS(x)  __attribute__ ((aligned( x )))
#endif

#if __cplusplus >= 201103
#define DEFINE_ALIGNED(def, a) alignas(a) def
#else
#if defined(_WIN32)
#define DEFINE_ALIGNED(def, a) __declspec(align(a)) def
#elif defined(__OSX__)
#define DEFINE_ALIGNED(def, a) def __attribute__((aligned(a)))
#else
//If we haven't specified the platform here, we fallback on the C++11 and C11 keyword for aligning
//Best case -> No platform specific align defined -> use this one that does the same thing
//Worst case -> No platform specific align defined -> this one also doesn't work and fails to compile -> add a platform specific one :)
#define DEFINE_ALIGNED(def, a) alignas(a) def
#endif
#endif

// Generates a compile error if the expression evaluates to false
#define COMPILE_ASSERT(exp) static_assert((exp), #exp)