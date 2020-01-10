#pragma once

//=============================================================================
SE_NAMESPACE_WND_BEGIN

enum MouseInput
{
	Left,
	Right,
	Middle,
	Button4,
	Button5,
	MouseInputMax
};

struct TouchPoint
{	
	unsigned long id;	// A unique id for each touch point	
	unsigned screenX;	// touch coordinate relative to whole screen origin in pixels	
	unsigned screenY;	// touch coordinate relative to whole screen origin in pixels	
	unsigned clientX;	// touch coordinate relative to window in pixels.	
	unsigned clientY;	// touch coordinate relative to window in pixels.	
	bool isChanged;		// Did the touch point change
};

SE_NAMESPACE_WND_END
//=============================================================================