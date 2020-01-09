#pragma once

#include "Platform/Window/Common/WindowDesc.h"
#include "Platform/Window/Win32/Win32EventQueue.h"

class ITaskbarList3;

//=============================================================================
SE_NAMESPACE_WND_BEGIN

class Window
{
public:
	~Window();

	bool create(WindowDesc& desc, EventQueue& eventQueue);

	const WindowDesc getDesc();

	void updateDesc(WindowDesc& desc);

	void setTitle(std::wstring title);

	void setPosition(unsigned x, unsigned y);

	void setMousePosition(unsigned x, unsigned y);

	void showMouse(bool show);

	void setSize(unsigned width, unsigned height);

	void setProgress(float progress);

	UVec2 getCurrentDisplaySize() const;

	// returns the current top left corner this window is located in
	UVec2 getCurrentDisplayPosition() const;

	UVec2 getWindowSize() const;

	std::string getTitle() const;

	void close();

	// Executes an event callback asynchronously, use this for non-blocking
	// events (resizing while rendering, etc.)
	void executeEventCallback(const Event e);

	static LRESULT CALLBACK WindowProcStatic(HWND hwnd, UINT msg, WPARAM wparam,
		LPARAM lparam);

	LRESULT WindowProc(UINT msg, WPARAM wparam, LPARAM lparam);

	// Application Handle
	HINSTANCE hinstance;

	// Window Handle
	HWND hwnd = nullptr;

	std::function<void(const Event e)> mCallback;

protected:
	EventQueue* mEventQueue;

	WindowDesc mDesc;

	// Window State
	WNDCLASSEX wndClass;

	// Window Size/Position
	RECT windowRect;

	// Screen State
	DEVMODE dmScreenSettings;

	// Window Behavior
	DWORD dwExStyle;
	DWORD dwStyle;

	// Taskbar Interface
	ITaskbarList3* mTaskbarList;
};

static thread_local Window* _windowBeingCreated = nullptr;
static thread_local std::unordered_map<HWND, Window*> _hwndMap = {};

typedef Window WindowDelegate;

SE_NAMESPACE_WND_END
//=============================================================================