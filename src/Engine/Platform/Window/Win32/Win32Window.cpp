#include "stdafx.h"
#include "Win32Window.h"
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
//-----------------------------------------------------------------------------
constexpr DWORD StyleWindowed = WS_OVERLAPPEDWINDOW;
constexpr DWORD StyleAeroBorderless = WS_POPUP | WS_THICKFRAME;
constexpr DWORD StyleBasicBorderless  = WS_POPUP | WS_VISIBLE;
//-----------------------------------------------------------------------------
HBRUSH hBrush = CreateSolidBrush(RGB(23, 26, 30));
//-----------------------------------------------------------------------------

//=============================================================================
SE_NAMESPACE_WND_BEGIN
//-----------------------------------------------------------------------------
static thread_local Window *sWindowBeingCreated = nullptr;
static thread_local std::unordered_map<HWND, Window*> sHwndMap = {};
//-----------------------------------------------------------------------------
Window::~Window()
{
	if ( hwnd != nullptr )
		Close();
}
//-----------------------------------------------------------------------------
bool Window::Create(WindowConfig &desc, EventQueue &eventQueue)
{
	m_eventQueue = &eventQueue;

	hinstance = GetModuleHandle(nullptr);

	m_desc = desc;

	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = Window::windowProcStatic;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = WS_EX_NOPARENTNOTIFY;
	wndClass.hInstance = hinstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = hBrush;
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = m_desc.name.c_str();
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if ( !RegisterClassEx(&wndClass) )
	{
		return false;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if ( m_desc.fullscreen )
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ( (desc.width != screenWidth) && (desc.height != screenHeight) )
		{
			if ( ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) !=
				DISP_CHANGE_SUCCESSFUL )
			{
			}
		}
	}

	DWORD dwExStyle = 0;
	DWORD dwStyle = 0;

	if ( m_desc.fullscreen )
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		//dwStyle = StyleBasicBorderless;
		dwStyle = StyleWindowed;
	}

	RECT windowRect;
	windowRect.left = m_desc.x;
	windowRect.top = m_desc.y;
	windowRect.right = m_desc.fullscreen ? (long)screenWidth : (long)desc.width;
	windowRect.bottom = m_desc.fullscreen ? (long)screenHeight : (long)desc.height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	sWindowBeingCreated = this;
	hwnd = CreateWindowEx(0, m_desc.name.c_str(), m_desc.title.c_str(), dwStyle, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, hinstance, NULL);
	if ( !hwnd )
	{
		return false;
	}

	BOOL isNCRenderingEnabled{ TRUE };
	DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_ENABLED, &isNCRenderingEnabled, sizeof(isNCRenderingEnabled));
	

	if ( !m_desc.fullscreen )
	{
		// Center on screen
		unsigned x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		unsigned y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if ( m_desc.visible )
	{
		ShowWindow(hwnd, SW_SHOW);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}

	static const DWM_BLURBEHIND blurBehind{ {0}, {TRUE}, {NULL}, {TRUE} };
	DwmEnableBlurBehindWindow(hwnd, &blurBehind);
	static const MARGINS shadow_state[2]{ {0, 0, 0, 0}, {1, 1, 1, 1} };
	::DwmExtendFrameIntoClientArea(hwnd, &shadow_state[0]);

	RegisterWindowMessage(L"TaskbarButtonCreated");
	HRESULT hrf = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (LPVOID*)&m_taskbarList);

	return true;
}
//-----------------------------------------------------------------------------
void Window::Close()
{
	if ( hwnd != nullptr )
	{
		DestroyWindow(hwnd);
		hwnd = nullptr;
	}
}
//-----------------------------------------------------------------------------
void Window::SetTitle(std::wstring_view title)
{
	m_desc.title = title;
	SetWindowText(hwnd, m_desc.title.c_str());
}
//-----------------------------------------------------------------------------
void Window::SetPosition(unsigned x, unsigned y)
{
	SetWindowPos(hwnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	m_desc.x = x;
	m_desc.y = y;
}
//-----------------------------------------------------------------------------
void Window::SetMousePosition(unsigned x, unsigned y)
{
	SetCursorPos(x, y);
}
//-----------------------------------------------------------------------------
void Window::SetSize(unsigned width, unsigned height)
{
	m_windowRect.left = m_desc.x;
	m_windowRect.top = m_desc.y;
	m_windowRect.right = (long)width;
	m_windowRect.bottom = (long)height;

	AdjustWindowRectEx(&m_windowRect, m_style, FALSE, m_exStyle);
}
//-----------------------------------------------------------------------------






//-----------------------------------------------------------------------------
const WindowConfig Window::GetDesc()
{
	return m_desc;
}
//-----------------------------------------------------------------------------






void Window::ShowMouse(bool show)
{
	ShowCursor(show ? TRUE : FALSE);
}

std::string Window::getTitle() const
{
	char str[1024];
	memset(str, 0, sizeof(char) * 1024);
	GetWindowTextA(hwnd, str, 1024);
	std::string outStr = std::string(str);
	return outStr;
}






UVec2 Window::getWindowSize() const
{
	LPRECT lpRect;
	GetWindowRect(hwnd, lpRect);
	return UVec2(lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
}

UVec2 Window::getCurrentDisplaySize() const
{
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	UVec2 r = UVec2(static_cast<unsigned>(screenWidth),
		static_cast<unsigned>(screenHeight));
	return r;
}

UVec2 Window::getCurrentDisplayPosition() const
{
	WINDOWPLACEMENT lpwndpl;
	GetWindowPlacement(hwnd, &lpwndpl);
	UVec2 r = UVec2(lpwndpl.ptMinPosition.x, lpwndpl.ptMinPosition.y);
	return r;
}



void Window::executeEventCallback(const Event e)
{
	if ( mCallback ) mCallback(e);
}

LRESULT CALLBACK Window::windowProcStatic(HWND hwnd, UINT msg, WPARAM wparam,
	LPARAM lparam)
{
	Window* _this;
	if ( sWindowBeingCreated != nullptr )
	{
		sHwndMap.emplace(hwnd, sWindowBeingCreated);
		sWindowBeingCreated->hwnd = hwnd;
		_this = sWindowBeingCreated;
		sWindowBeingCreated = nullptr;
	}
	else
	{
		auto existing = sHwndMap.find(hwnd);
		_this = existing->second;
	}

	return _this->WindowProc(msg, wparam, lparam);
}

LRESULT Window::WindowProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
	MSG message;
	message.hwnd = hwnd;
	message.lParam = lparam;
	message.wParam = wparam;
	message.message = msg;
	message.time = 0;

	LRESULT result = m_eventQueue->pushEvent(message, this);
	if ( result > 0 ) return result;
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
SE_NAMESPACE_WND_END
//=============================================================================