#pragma once

// Architecture
#define SE_ARCH_32BIT              0
#define SE_ARCH_64BIT              0

// Debug
#define SE_DEBUG                   0

// Compiler
#define SE_COMPILER_MSVC           0
#define SE_COMPILER_CLANG          0
#define SE_COMPILER_CLANG_ANALYZER 0
#define SE_COMPILER_GNUC           0
#define SE_COMPILER_INTEL          0
#define SE_COMPILER_MINGW          0

//=============================================================================
// Architecture
//=============================================================================
#if defined(__x86_64__)    || \
	defined(_M_X64)        || \
	defined(__aarch64__)   || \
	defined(__64BIT__)     || \
	defined(__mips64)      || \
	defined(__powerpc64__) || \
	defined(__ppc64__)     || \
	defined(__LP64__)
#	undef  SE_ARCH_64BIT
#	define SE_ARCH_64BIT 64
#else
#	undef  SE_ARCH_32BIT
#	define SE_ARCH_32BIT 32
#endif

//=============================================================================
// DEBUG MODE
//=============================================================================
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#	undef  SE_DEBUG
#	define SE_DEBUG 1
#endif

//=============================================================================
// Compiler
//=============================================================================
#if defined(__clang__)
#	undef  SE_COMPILER_CLANG
#	define SE_COMPILER_CLANG __clang_version__
#	if defined(__clang_analyzer__)
#		undef  SE_COMPILER_CLANG_ANALYZER
#		define SE_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(__GNUC__) // Check after Clang, as Clang defines this too
#	undef  SE_COMPILER_GNUC
#	define SE_COMPILER_GNUC (((__GNUC__)*100) + (__GNUC_MINOR__*10) + __GNUC_PATCHLEVEL__)
#elif defined(__INTEL_COMPILER)
#	undef  SE_COMPILER_INTEL
#	define SE_COMPILER_INTEL __INTEL_COMPILER
#elif defined(__MINGW32__) || defined(__MINGW64__)
#	undef  SE_COMPILER_MINGW
#	define SE_COMPILER_MINGW 1
#elif defined(_MSC_VER) && (_MSC_VER >= 1900) // Check after Clang and Intel, since we could be building with either within VS
#	undef  SE_COMPILER_MSVC
#	define SE_COMPILER_MSVC _MSC_VER
#else
#	error "Unknown compiler."
#endif