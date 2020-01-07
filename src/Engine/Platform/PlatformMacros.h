#pragma once

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