#pragma once

//=============================================================================
// Namespace Window
//=============================================================================
#define SE_NAMESPACE_WND               window
#define SE_NAMESPACE_WND_BEGIN         namespace SE_NAMESPACE::SE_NAMESPACE_WND {
#define SE_NAMESPACE_WND_END           }
namespace SE_NAMESPACE::SE_NAMESPACE_WND{}

//=============================================================================
// Platform Macros
//=============================================================================
#if SE_COMPILER_MSVC
#	define SE_NO_VTABLE __declspec(novtable) // https://habr.com/ru/post/442340/
#elif SE_COMPILER_CLANG
#	define SE_NO_VTABLE
#elif SE_COMPILER_GNUC
#	define SE_NO_VTABLE
#endif