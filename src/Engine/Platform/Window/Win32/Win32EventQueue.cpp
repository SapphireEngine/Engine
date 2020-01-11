#include "stdafx.h"
#include "Win32EventQueue.h"
#include "Win32Window.h"
//-----------------------------------------------------------------------------
#ifndef HID_USAGE_PAGE_GENERIC
#   define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#   define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif
//-----------------------------------------------------------------------------
RAWINPUTDEVICE Rid[1];
//-----------------------------------------------------------------------------
// some sizing border definitions
#define MINX 200
#define MINY 200
#define BORDERWIDTH 5
#define TITLEBARWIDTH 5

//=============================================================================
SE_NAMESPACE_WND_BEGIN
//-----------------------------------------------------------------------------
void EventQueue::Update()
{
    MSG msg = {};

    while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
    {
        // Translate virtual key messages
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
//-----------------------------------------------------------------------------
const Event& EventQueue::Front()
{
    return m_queue.front();
}
//-----------------------------------------------------------------------------
void EventQueue::Pop()
{
    m_queue.pop();
}
//-----------------------------------------------------------------------------
bool EventQueue::Empty()
{
    return m_queue.empty();
}
//-----------------------------------------------------------------------------
size_t EventQueue::Size()
{
    return m_queue.size();
}
//-----------------------------------------------------------------------------
LRESULT EventQueue::PushEvent(MSG msg, Window *window)
{
    UINT message = msg.message;
    LRESULT result = 0;
    // TODO: hwnd to Window unordered_map, when Window closes, it sends a message to the event queue to remove that hwnd and any remaining events that match that Window

    if ( !m_initialized )
    {
        m_initialized = true;
        Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
        Rid[0].dwFlags = RIDEV_INPUTSINK;
        Rid[0].hwndTarget = window->hwnd;
        RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
    }

    Event e = Event(EventType::None, window);

    switch ( message )
    {
    case WM_CREATE:
        {
            e = Event(EventType::Create, window);
            //repaint window when borderless to avoid 6px resizable border.
            CREATESTRUCT *WindowInfo = reinterpret_cast<CREATESTRUCT*>(msg.lParam);
            MoveWindow(window->hwnd, WindowInfo->x, WindowInfo->y, WindowInfo->cx - BORDERWIDTH, WindowInfo->cy - BORDERWIDTH, TRUE);
            break;
        }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(window->hwnd, &ps);

            RECT ClientRect;
            GetClientRect(window->hwnd, &ClientRect);
            RECT BorderRect = { BORDERWIDTH, BORDERWIDTH,
                ClientRect.right - BORDERWIDTH - BORDERWIDTH,
                ClientRect.bottom - BORDERWIDTH - BORDERWIDTH },
                TitleRect = { BORDERWIDTH, BORDERWIDTH,
                    ClientRect.right - BORDERWIDTH - BORDERWIDTH,
                    TITLEBARWIDTH };

            HBRUSH BorderBrush = CreateSolidBrush(RGB(23, 26, 30));
            FillRect(ps.hdc, &ClientRect, BorderBrush);
            FillRect(ps.hdc, &BorderRect, BorderBrush);
            FillRect(ps.hdc, &TitleRect, BorderBrush);
            DeleteObject(BorderBrush);

            EndPaint(window->hwnd, &ps);
            e = Event(EventType::Paint, window);
            break;
        }
    case WM_ERASEBKGND:
        {
            break;
        }
    case WM_DESTROY:
        {
            e = Event(EventType::Close, window);
            break;
        }
    case WM_SETFOCUS:
        {
            e = Event(FocusData(true), window);
            break;
        }
    case WM_KILLFOCUS:
        {
            e = Event(FocusData(false), window);
            break;
        }
    case WM_NCHITTEST:
        {
            RECT WindowRect;
            int x, y;

            GetWindowRect(window->hwnd, &WindowRect);
            x = GET_X_LPARAM(msg.lParam) - WindowRect.left;
            y = GET_Y_LPARAM(msg.lParam) - WindowRect.top;

            if ( x >= BORDERWIDTH &&
                x <= WindowRect.right - WindowRect.left - BORDERWIDTH &&
                y >= BORDERWIDTH && y <= TITLEBARWIDTH )
                result = HTCAPTION;
            else if ( x < BORDERWIDTH && y < BORDERWIDTH )
                result = HTTOPLEFT;
            else if ( x > WindowRect.right - WindowRect.left - BORDERWIDTH &&
                y < BORDERWIDTH )
                result = HTTOPRIGHT;
            else if ( x > WindowRect.right - WindowRect.left - BORDERWIDTH &&
                y > WindowRect.bottom - WindowRect.top - BORDERWIDTH )
                result = HTBOTTOMRIGHT;
            else if ( x < BORDERWIDTH &&
                y > WindowRect.bottom - WindowRect.top - BORDERWIDTH )
                result = HTBOTTOMLEFT;
            else if ( x < BORDERWIDTH )
                result = HTLEFT;
            else if ( y < BORDERWIDTH )
                result = HTTOP;
            else if ( x > WindowRect.right - WindowRect.left - BORDERWIDTH )
                result = HTRIGHT;
            else if ( y > WindowRect.bottom - WindowRect.top - BORDERWIDTH )
                result = HTBOTTOM;
            else
                result = HTCLIENT;
            break;
        }
    case WM_MOUSEWHEEL:
        {
            short modifiers = LOWORD(msg.wParam);
            e = Event(
                MouseWheelData(
                    GET_WHEEL_DELTA_WPARAM(msg.wParam) / WHEEL_DELTA,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_LBUTTONDOWN:
        {
            short modifiers = LOWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    MouseInput::Left, ButtonState::Pressed,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_MBUTTONDOWN:
        {
            short modifiers = LOWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    MouseInput::Middle, ButtonState::Pressed,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_RBUTTONDOWN:
        {
            short modifiers = LOWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    MouseInput::Right, ButtonState::Pressed,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_XBUTTONDOWN:
        {
            short modifiers = LOWORD(msg.wParam);
            short x = HIWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    x & XBUTTON1 ? MouseInput::Button4 : MouseInput::Button5,
                    ButtonState::Pressed,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_XBUTTONUP:
        {
            short modifiers = LOWORD(msg.wParam);
            short x = HIWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    x & XBUTTON1 ? MouseInput::Button4 : MouseInput::Button5,
                    ButtonState::Released,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_LBUTTONDBLCLK:
        // Perhaps there should be an event for this in the future
        break;
    case WM_LBUTTONUP:
        {
            short modifiers = LOWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    MouseInput::Left, ButtonState::Released,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_MBUTTONUP:
        {
            short modifiers = LOWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    MouseInput::Middle, ButtonState::Released,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_RBUTTONUP:
        {
            short modifiers = LOWORD(msg.wParam);
            e = Event(
                MouseInputData(
                    MouseInput::Right, ButtonState::Released,
                    ModifierState(modifiers & MK_CONTROL, modifiers & MK_ALT,
                        modifiers & MK_SHIFT, modifiers & 0)),
                window);
            break;
        }
    case WM_INPUT:
        {
            UINT dwSize;

            GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize,
                sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if ( lpb == NULL )
            {
                // return 0;
            }

            if ( GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize,
                sizeof(RAWINPUTHEADER)) != dwSize )
                OutputDebugString(
                    TEXT("GetRawInputData does not return correct size !\n"));

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if ( raw->header.dwType == RIM_TYPEKEYBOARD )
            {

                raw->data.keyboard.MakeCode, raw->data.keyboard.Flags,
                    raw->data.keyboard.Reserved,
                    raw->data.keyboard.ExtraInformation, raw->data.keyboard.Message,
                    raw->data.keyboard.VKey;
            }
            else if ( raw->header.dwType == RIM_TYPEMOUSE )
            {

                raw->data.mouse.usFlags, raw->data.mouse.ulButtons,
                    raw->data.mouse.usButtonFlags, raw->data.mouse.usButtonData,
                    raw->data.mouse.ulRawButtons, raw->data.mouse.lLastX,
                    raw->data.mouse.lLastY, raw->data.mouse.ulExtraInformation;

                e = Event(
                    MouseRawData(static_cast<int>(raw->data.mouse.lLastX),
                        static_cast<int>(raw->data.mouse.lLastY)),
                    window);
            }

            delete[] lpb;
            break;
        }
    case WM_MOUSEMOVE:
        {
            HWND hwnd = window->hwnd;
            // Extract the mouse local coordinates
            int x = static_cast<short>(LOWORD(msg.lParam));
            int y = static_cast<short>(HIWORD(msg.lParam));

            // Get the client area of the window
            RECT area;
            GetClientRect(hwnd, &area);
            
            e = Event(MouseMoveData(
                static_cast<unsigned>(
                    area.left <= x && x <= area.right ? x - area.left
                    : 0xFFFFFFFF),
                static_cast<unsigned>(
                    area.top <= y && y <= area.bottom ? y - area.top
                    : 0xFFFFFFFF),
                static_cast<unsigned>(x), static_cast<unsigned>(y),
                static_cast<int>(x - m_prevMouseX),
                static_cast<int>(y - m_prevMouseY)),
                window);
            m_prevMouseX = static_cast<unsigned>(x);
            m_prevMouseY = static_cast<unsigned>(y);
            break;
        }
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_CHAR:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        {
            Key d;

            switch ( msg.wParam )
            {
            case VK_ESCAPE:
                d = Key::Escape;
                break;
            case 0x30:
                d = Key::Num0;
                break;
            case 0x31:
                d = Key::Num1;
                break;
            case 0x32:
                d = Key::Num2;
                break;
            case 0x33:
                d = Key::Num3;
                break;
            case 0x34:
                d = Key::Num4;
                break;
            case 0x35:
                d = Key::Num5;
                break;
            case 0x36:
                d = Key::Num6;
                break;
            case 0x37:
                d = Key::Num7;
                break;
            case 0x38:
                d = Key::Num8;
                break;
            case 0x39:
                d = Key::Num9;
                break;
            case 0x41:
                d = Key::A;
                break;
            case 0x42:
                d = Key::B;
                break;
            case 0x43:
                d = Key::C;
                break;
            case 0x44:
                d = Key::D;
                break;
            case 0x45:
                d = Key::E;
                break;
            case 0x46:
                d = Key::F;
                break;
            case 0x47:
                d = Key::G;
                break;
            case 0x48:
                d = Key::H;
                break;
            case 0x49:
                d = Key::I;
                break;
            case 0x4A:
                d = Key::J;
                break;
            case 0x4B:
                d = Key::K;
                break;
            case 0x4C:
                d = Key::L;
                break;
            case 0x4D:
                d = Key::M;
                break;
            case 0x4E:
                d = Key::N;
                break;
            case 0x4F:
                d = Key::O;
                break;
            case 0x50:
                d = Key::P;
                break;
            case 0x51:
                d = Key::Q;
                break;
            case 0x52:
                d = Key::R;
                break;
            case 0x53:
                d = Key::S;
                break;
            case 0x54:
                d = Key::T;
                break;
            case 0x55:
                d = Key::U;
                break;
            case 0x56:
                d = Key::V;
                break;
            case 0x57:
                d = Key::W;
                break;
            case 0x58:
                d = Key::X;
                break;
            case 0x59:
                d = Key::Y;
                break;
            case 0x5A:
                d = Key::Z;
                break;
            case VK_NUMPAD0:
                d = Key::Numpad0;
                break;
            case VK_NUMPAD1:
                d = Key::Numpad1;
                break;
            case VK_NUMPAD2:
                d = Key::Numpad2;
                break;
            case VK_NUMPAD3:
                d = Key::Numpad3;
                break;
            case VK_NUMPAD4:
                d = Key::Numpad4;
                break;
            case VK_NUMPAD5:
                d = Key::Numpad5;
                break;
            case VK_NUMPAD6:
                d = Key::Numpad6;
                break;
            case VK_NUMPAD7:
                d = Key::Numpad7;
                break;
            case VK_NUMPAD8:
                d = Key::Numpad8;
                break;
            case VK_NUMPAD9:
                d = Key::Numpad9;
                break;
            case VK_UP:
                d = Key::Up;
                break;
            case VK_LEFT:
                d = Key::Left;
                break;
            case VK_DOWN:
                d = Key::Down;
                break;
            case VK_RIGHT:
                d = Key::Right;
                break;
            case VK_SPACE:
                d = Key::Space;
                break;
            case VK_HOME:
                d = Key::Home;
                break;
            case VK_F1:
                d = Key::F1;
                break;
            case VK_F2:
                d = Key::F2;
                break;
            case VK_F3:
                d = Key::F3;
                break;
            case VK_F4:
                d = Key::F4;
                break;
            case VK_F5:
                d = Key::F5;
                break;
            case VK_F6:
                d = Key::F6;
                break;
            case VK_F7:
                d = Key::F7;
                break;
            case VK_F8:
                d = Key::F8;
                break;
            case VK_F9:
                d = Key::F9;
                break;
            case VK_F10:
                d = Key::F10;
                break;
            case VK_F11:
                d = Key::F11;
                break;
            case VK_F12:
                d = Key::F12;
                break;

            default:
                d = Key::KeysMax;
                break;
            }

            if ( message == WM_KEYDOWN || message == WM_SYSKEYDOWN )
            {
                e = Event(
                    KeyboardData(d, ButtonState::Pressed, ModifierState()), window);
            }
            else if ( message == WM_KEYUP || message == WM_SYSKEYUP )
            {
                e = Event(
                    KeyboardData(d, ButtonState::Released, ModifierState()),
                    window);
            }
            break;
        }
    case WM_SIZE:
        {
            unsigned width, height;
            width = static_cast<unsigned>((UINT64)msg.lParam & 0xFFFF);
            height = static_cast<unsigned>((UINT64)msg.lParam >> 16);

            e = Event(ResizeData(width, height, false), window);
            break;
        }
    case WM_SIZING:
        {
            unsigned WIDTH = 320;
            unsigned HEIGHT = 320;
            unsigned width, height;
            unsigned STEP = 1;
            PRECT rectp = (PRECT)msg.lParam;
            HWND hwnd = window->hwnd;
            // Get the window and client dimensions
            tagRECT wind, rect;
            GetWindowRect(hwnd, &wind);
            GetClientRect(hwnd, &rect);
            width = rectp->right - rectp->left;
            height = rectp->bottom - rectp->top;

            RedrawWindow(hwnd, NULL, NULL,
                RDW_INVALIDATE | RDW_NOERASE | RDW_INTERNALPAINT);

            e = Event(ResizeData(width, height, true), window);
            break;
        }
    case WM_NCCALCSIZE:
        {
            if ( msg.lParam )
            {
                NCCALCSIZE_PARAMS* sz = (NCCALCSIZE_PARAMS*)msg.lParam;
                if ( msg.wParam )
                {
                    sz->rgrc[0].bottom +=
                        BORDERWIDTH; // rgrc[0] is what makes this work, don't know
                                    // what others (rgrc[1], rgrc[2]) do, but why
                                    // not change them all?
                    sz->rgrc[0].right += BORDERWIDTH;
                    sz->rgrc[1].bottom += BORDERWIDTH;
                    sz->rgrc[1].right += BORDERWIDTH;
                    sz->rgrc[2].bottom += BORDERWIDTH;
                    sz->rgrc[2].right += BORDERWIDTH;
                    return 0;
                }
            }
            break;
        }
    case WM_GETMINMAXINFO: // It is used to restrict WS_POPUP window size
        {                      // I don't know if this works on others
            MINMAXINFO* min_max = reinterpret_cast<MINMAXINFO*>(msg.lParam);

            min_max->ptMinTrackSize.x = MINX;
            min_max->ptMinTrackSize.y = MINY;
            break;
        }
    default:
        // Do nothing
        break;
    }
    if ( e.type != EventType::None )
    {
        m_queue.emplace(e);
    }
    window->executeEventCallback(e);
    return result;
}

SE_NAMESPACE_WND_END
//=============================================================================