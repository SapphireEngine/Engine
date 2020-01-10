#include "stdafx.h"
#include "Event.h"

//=============================================================================
SE_NAMESPACE_WND_BEGIN
//-----------------------------------------------------------------------------
Event::Event(EventType type, Window *window) 
	: type(type), window(window)
{
}
//-----------------------------------------------------------------------------
Event::Event(FocusData d, Window *window)
	: type(EventType::Focus), window(window)
{
	data.focus = d;
}
//-----------------------------------------------------------------------------
Event::Event(ResizeData d, Window *window)
	: type(EventType::Resize), window(window)
{
	data.resize = d;
}
//-----------------------------------------------------------------------------
Event::Event(KeyboardData d, Window *window)
	: type(EventType::Keyboard), window(window)
{
	data.keyboard = d;
}
//-----------------------------------------------------------------------------
Event::Event(MouseRawData d, Window *window)
	: type(EventType::MouseRaw), window(window)
{
	data.mouseRaw = d;
}
//-----------------------------------------------------------------------------
Event::Event(MouseMoveData d, Window *window)
	: type(EventType::MouseMoved), window(window)
{
	data.mouseMoved = d;
}
//-----------------------------------------------------------------------------
Event::Event(MouseInputData d, Window *window)
	: type(EventType::MouseInput), window(window)
{
	data.mouseInput = d;
}
//-----------------------------------------------------------------------------
Event::Event(MouseWheelData d, Window *window)
	: type(EventType::MouseWheel), window(window)
{
	data.mouseWheel = d;
}
//-----------------------------------------------------------------------------
Event::Event(TouchData d, Window *window)
	: type(EventType::Touch), window(window)
{
	data.touch = d;
}
//-----------------------------------------------------------------------------
Event::Event(GamepadData d, Window *window)
	: type(EventType::Gamepad), window(window)
{
	data.gamepad = d;
}
//-----------------------------------------------------------------------------

SE_NAMESPACE_WND_END
//=============================================================================