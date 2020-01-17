#pragma once

#include "Platform/Window/Common/WindowConfig.h"
#include "Platform/Window/Common/Event.h"
#include "Core/Object/Subsystem.h"

struct ITaskbarList3;

//=============================================================================
SE_NAMESPACE_WND_BEGIN

class EventQueue;

struct WindowData
{
	HINSTANCE hinstance;
	HWND hwnd = nullptr;
};

class IRenderWindow;
struct WindowHandle final
{
	handle nativeWindowHandle;
	IRenderWindow *renderWindow;
};

class Window : public Subsystem<Window>
{
	friend class EventQueue;
public:
	Window();
	~Window();

	bool Init(WindowConfig &desc, EventQueue &eventQueue);
	void Close();

	void ShowMouse(bool show);

	void SetTitle(std::wstring_view title);
	void SetPosition(unsigned x, unsigned y);
	void SetMousePosition(unsigned x, unsigned y);
	void SetSize(unsigned width, unsigned height);

	const WindowConfig GetDesc();	
	UVec2 GetCurrentDisplaySize() const;	
	UVec2 GetCurrentDisplayPosition() const; // returns the current top left corner this window is located in
	UVec2 GetWindowSize() const;

	HINSTANCE hinstance;
	HWND hwnd = nullptr;

	std::function<void(const Event e)> mCallback;

protected:
	static LRESULT CALLBACK windowProcStatic(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	// Executes an event callback asynchronously, use this for non-blocking events (resizing while rendering, etc.)
	LRESULT windowProc(UINT msg, WPARAM wparam, LPARAM lparam);
	void executeEventCallback(const Event e);


	EventQueue *m_eventQueue;
	WindowConfig m_desc;		
	RECT m_windowRect;			// Window Size/Position	
	DEVMODE m_screenSettings;	// Screen State

	// Window Behavior
	DWORD m_exStyle;
	DWORD m_style;
	
	ITaskbarList3 *m_taskbarList;// Taskbar Interface
};

SE_NAMESPACE_WND_END
//=============================================================================