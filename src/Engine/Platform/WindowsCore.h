#pragma once

#include "Math/MathTypes.h"

//=============================================================================
SE_NAMESPACE_BEGIN

typedef void* IconHandle;

typedef struct WindowHandle
{
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	Display*                 display;
	Window                   window;
	Atom                     xlib_wm_delete_window;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	xcb_connection_t*        connection;
	xcb_window_t             window;
	xcb_screen_t*            screen;
	xcb_intern_atom_reply_t* atom_wm_delete_window;
#else
	void*                    window;    //hWnd
#endif
} WindowHandle;

typedef struct RectDesc
{
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;
} RectDesc;

inline int getRectWidth(const RectDesc& rect)
{
	return rect.right - rect.left;
}

inline int getRectHeight(const RectDesc& rect)
{
	return rect.bottom - rect.top;
}

struct WindowsDesc;

struct WindowCallbacks
{
	void    (*onResize)(WindowsDesc* window, int32_t newSizeX, int32_t newSizeY);
	int32_t(*onHandleMessage)(WindowsDesc* window, void* msg);
};

typedef struct WindowsDesc
{
	WindowHandle              handle;    //hWnd

	WindowCallbacks           callbacks;

	RectDesc                  windowedRect;
	RectDesc                  fullscreenRect;
	RectDesc                  clientRect;
	IconHandle                bigIcon;
	IconHandle                smallIcon;

	uint32_t                  windowsFlags;
	bool                      fullScreen;
	bool                      cursorTracked;
	bool                      iconified;
	bool                      maximized;
	bool                      minimized;
	bool                      hide;
} WindowsDesc;

typedef struct Resolution
{
	uint32_t m_width;
	uint32_t m_height;
} Resolution;

// Monitor data
//
typedef struct MonitorDesc
{
	RectDesc monitorRect;
	RectDesc workRect;
	// This size matches the static size of DISPLAY_DEVICE.DeviceName
#ifdef _WIN32
	WCHAR adapterName[32];
	WCHAR displayName[32];
	WCHAR publicAdapterName[64];
	WCHAR publicDisplayName[64];
#else
	char  adapterName[32];
	char  displayName[32];
	char  publicAdapterName[64];
	char  publicDisplayName[64];
#endif
	bool modesPruned;
	bool modeChanged;

	Resolution  defaultResolution;
	Resolution* resolutions;
	uint32_t    resolutionCount;
} MonitorDesc;

// Define some sized types
typedef uint8_t uint8;
typedef int8_t  int8;

typedef uint16_t uint16;
typedef int16_t  int16;

typedef uint32_t uint32;
typedef int32_t  int32;

typedef ptrdiff_t intptr;

#ifdef _WIN32
typedef signed __int64   int64;
typedef unsigned __int64 uint64;
#elif defined(__linux__)
typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef signed long long   int64;
typedef unsigned long long uint64;
#endif

typedef uint8        ubyte;
typedef uint16       ushort;
typedef unsigned int uint;
typedef const char *LPCSTR, *PCSTR;

// API functions
void requestShutdown();

// Window handling
void openWindow(const char* app_name, WindowsDesc* winDesc);
void closeWindow(const WindowsDesc* winDesc);
void setWindowRect(WindowsDesc* winDesc, const RectDesc& rect);
void setWindowSize(WindowsDesc* winDesc, unsigned width, unsigned height);
void toggleFullscreen(WindowsDesc* winDesc);
void showWindow(WindowsDesc* winDesc);
void hideWindow(WindowsDesc* winDesc);
void maximizeWindow(WindowsDesc* winDesc);
void minimizeWindow(WindowsDesc* winDesc);

void setMousePositionRelative(const WindowsDesc* winDesc, int32_t x, int32_t y);

void getRecommendedResolution(RectDesc* rect);
// Sets video mode for specified display
void setResolution(const MonitorDesc* pMonitor, const Resolution* pRes);

MonitorDesc* getMonitor(uint32_t index);
float2       getDpiScale();

bool getResolutionSupport(const MonitorDesc* pMonitor, const Resolution* pRes);

// Shell commands

typedef struct Path Path;

/// @param stdOutFile The file to which the output of the command should be written. May be NULL.
int systemRun(const char *command, const char **arguments, size_t argumentCount, const Path* stdOutFile);

class IApp;
int WindowsMain(int argc, char** argv, IApp* app);

SE_NAMESPACE_END
//=============================================================================