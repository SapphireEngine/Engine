#pragma once

#include "Platform/Window/Common/WindowConfig.h"
#include "Platform/Window/Win32/Win32EventQueue.h"

struct ITaskbarList3;

//=============================================================================
SE_NAMESPACE_WND_BEGIN

class Window
{
public:
	~Window();

	bool Create(WindowConfig &desc, EventQueue &eventQueue);
	void Close();

	void SetTitle(std::wstring_view title);
	void SetPosition(unsigned x, unsigned y);
	void SetMousePosition(unsigned x, unsigned y);
	void SetSize(unsigned width, unsigned height);




	const WindowConfig GetDesc();


	



	void ShowMouse(bool show);
	
	UVec2 getCurrentDisplaySize() const;

	// returns the current top left corner this window is located in
	UVec2 getCurrentDisplayPosition() const;

	UVec2 getWindowSize() const;

	std::string getTitle() const;

	

	// Executes an event callback asynchronously, use this for non-blocking
	// events (resizing while rendering, etc.)
	void executeEventCallback(const Event e);	

	LRESULT WindowProc(UINT msg, WPARAM wparam, LPARAM lparam);

	// Application Handle
	HINSTANCE hinstance;

	// Window Handle
	HWND hwnd = nullptr;

	std::function<void(const Event e)> mCallback;

protected:
	static LRESULT CALLBACK windowProcStatic(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


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