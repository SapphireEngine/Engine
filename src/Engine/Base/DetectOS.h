#pragma once

// Platform OS
#define SE_PLATFORM_ANDROID        0
#define SE_PLATFORM_EMSCRIPTEN     0
#define SE_PLATFORM_LINUX          0
#define SE_PLATFORM_WINDOWS        0
#define SE_PLATFORM_WINRT          0

//=============================================================================
// Platform
//=============================================================================
#if defined(_WIN32) || defined(_WIN64)
#	undef  SE_PLATFORM_WINDOWS
#	define SE_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  SE_PLATFORM_ANDROID
#	define SE_PLATFORM_ANDROID __ANDROID_API__
#elif  defined(__linux__)
#	undef  SE_PLATFORM_LINUX
#	define SE_PLATFORM_LINUX 1
#elif defined(__EMSCRIPTEN__)
#	undef  SE_PLATFORM_EMSCRIPTEN
#	define SE_PLATFORM_EMSCRIPTEN 1
#else
#	error Unknown platform.
#endif