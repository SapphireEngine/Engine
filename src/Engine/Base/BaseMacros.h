#pragma once

//=============================================================================
// Namespace
//=============================================================================
#define SE_NAMESPACE                   se
#define SE_NAMESPACE_BEGIN             namespace SE_NAMESPACE {
#define SE_NAMESPACE_END               }
namespace SE_NAMESPACE {}

//=============================================================================
// Macros Utility
//=============================================================================
#define SE_STRINGIZE( _n ) SE_STRINGIZE_2( _n )
#define SE_STRINGIZE_2( _n ) #_n

#define SE_NULL_HANDLE 0

//=============================================================================
// Macros TODO
//=============================================================================
#if SE_COMPILER_MSVC
#	define TODO( _msg ) __pragma(message("" __FILE__ "(" SE_STRINGIZE(__LINE__) "): TODO: " _msg))
#else
#	define TODO( _msg )
#endif

//=============================================================================
// Pragma Warning
//=============================================================================
#if SE_COMPILER_MSVC
#	define SE_PRAGMA_WARNING_PUSH __pragma(warning(push))
#	define SE_PRAGMA_WARNING_POP __pragma(warning(pop))
#	define SE_PRAGMA_WARNING_DISABLE_MSVC(id) __pragma(warning(disable: id))
#	define SE_PRAGMA_WARNING_DISABLE_CLANG(id)
#	define SE_PRAGMA_WARNING_DISABLE_GCC(id)
#elif SE_COMPILER_CLANG
#	define SE_PRAGMA_WARNING_PUSH _Pragma("clang diagnostic push")
#	define SE_PRAGMA_WARNING_POP _Pragma("clang diagnostic pop")
#	define SE_PRAGMA_WARNING_DISABLE_MSVC(id)
#	define SE_PRAGMA_WARNING_DISABLE_CLANG(id) _Pragma(SE_STRINGIZE_2(clang diagnostic ignored id) )
#	define SE_PRAGMA_WARNING_DISABLE_GCC(id)
#elif SE_COMPILER_GNUC
#	define SE_PRAGMA_WARNING_PUSH _Pragma("GCC diagnostic push")
#	define SE_PRAGMA_WARNING_POP _Pragma("GCC diagnostic pop")
#	define SE_PRAGMA_WARNING_DISABLE_MSVC(id)
#	define SE_PRAGMA_WARNING_DISABLE_CLANG(id)
#	define SE_PRAGMA_WARNING_DISABLE_GCC(id) _Pragma(SE_STRINGIZE_2(GCC diagnostic ignored id) )
#endif

//=============================================================================
// Debug Parameter
//=============================================================================
// Resource name for debugging purposes, ignored when not using "SE_DEBUG"
#ifdef SE_DEBUG
#	define SE_DEBUG_NAME_PARAMETER , const char debugName[] = ""
#	define SE_DEBUG_NAME_PARAMETER_NO_DEFAULT , const char debugName[]
#	define SE_DEBUG_PASS_PARAMETER , debugName
#else
#	define SE_DEBUG_NAME_PARAMETER
#	define SE_DEBUG_NAME_PARAMETER_NO_DEFAULT
#	define SE_DEBUG_PASS_PARAMETER
#endif