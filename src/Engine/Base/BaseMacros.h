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

//=============================================================================
// Macros TODO
//=============================================================================
#if SE_COMPILER_MSVC
#	define TODO( _msg ) __pragma(message("" __FILE__ "(" SE_STRINGIZE(__LINE__) "): TODO: " _msg))
#else
#	define TODO( _msg )
#endif