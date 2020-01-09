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

// Touch point data
struct TouchPoint
{
	// A unique id for each touch point
	unsigned long id;

	// touch coordinate relative to whole screen origin in pixels
	unsigned screenX;

	// touch coordinate relative to whole screen origin in pixels
	unsigned screenY;

	// touch coordinate relative to window in pixels.
	unsigned clientX;

	// touch coordinate relative to window in pixels.
	unsigned clientY;

	// Did the touch point change
	bool isChanged;
};

SE_NAMESPACE_WND_END
//=============================================================================