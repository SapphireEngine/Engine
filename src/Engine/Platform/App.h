#pragma once

#include "Platform/WindowsCore.h"

//=============================================================================
SE_NAMESPACE_BEGIN

class IApp
{
public:
	virtual bool Init() = 0;
	virtual void Exit() = 0;

	virtual bool Load() = 0;
	virtual void Unload() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;

	virtual const char* GetName() = 0;

	struct Settings
	{
		/// Window width
		int32_t  mWidth = -1;
		/// Window height
		int32_t  mHeight = -1;
		/// Set to true if fullscreen mode has been requested
		bool     mFullScreen = false;
		/// Set to true if app wants to use an external window
		bool     mExternalWindow = false;
#if defined(TARGET_IOS)
		bool     mShowStatusBar = false;
		float    mContentScaleFactor = 0.f;
#endif
	} mSettings;

	WindowsDesc*    pWindow;
	const char*     pCommandLine;

	static int          argc;
	static const char** argv;
};

SE_NAMESPACE_END
//=============================================================================