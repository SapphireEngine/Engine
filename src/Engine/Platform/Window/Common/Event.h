#pragma once

#include "Platform/Window/Common/Key.h"
#include "Platform/Window/Common/Mouse.h"
#include "Platform/Window/Common/Gamepad.h"

//=============================================================================
SE_NAMESPACE_WND_BEGIN

class Window;

enum class EventType : size_t
{
	None = 0,
	// Closing a window
	Close,
	// Creating a window
	Create,
	// Focus/Unfocus on a window
	Focus,
	// Paint events, a good time to present any graphical changes
	Paint,
	// Resizing a window
	Resize,
	// Change in the screen DPI scaling (such as moving a window to a monitor with a larger DPI.
	DPI,
	// Keyboard input such as press/release events
	Keyboard,
	// Mouse moving events
	MouseMoved,
	// Raw mouse data events
	MouseRaw,
	// Mouse scrolling events
	MouseWheel,
	// Mouse button press events
	MouseInput,
	// Touch events
	Touch,
	// Gamepad Input Events such as analog sticks, button presses
	Gamepad,
	// Dropping a file on the window
	DropFile,
	// Hovering a file over a window
	HoverFile,
	EventTypeMax
};

/**
* Focus data passed with Focus events
*/
struct FocusData
{
	// true if focused, false if not
	bool focused;

	FocusData(bool focused);

	static const EventType type = EventType::Focus;
};

/**
* Resize data passed with Resize events
*/
struct ResizeData
{
	// new width of window viewport
	unsigned width;

	// New height of window viewport
	unsigned height;

	// In the process of resizing
	bool resizing;

	ResizeData(unsigned width, unsigned height, bool resizing);

	static const EventType type = EventType::Resize;
};

/**
* DPI data passed with DPI events
*/
struct DPIData
{
	float scale;

	static const EventType type = EventType::DPI;
};

/**
* Data sent during keyboard events
*/
struct KeyboardData
{
	Key key;
	ButtonState state;
	ModifierState modifiers;

	static const EventType type = EventType::Keyboard;

	KeyboardData(Key key, ButtonState state, ModifierState modifiers);
};

struct MouseRawData
{
	int deltax;
	int deltay;

	static const EventType type = EventType::MouseRaw;

	MouseRawData(int deltax, int deltay);
};

/**
* The event data passed with mouse events click, mouse moving events
*/
struct MouseMoveData
{
	// Current x position relative to active window
	unsigned x;

	// Current y position relative to active window
	unsigned y;

	// Current global x position
	unsigned screenx;

	// Current in global y position
	unsigned screeny;

	// Change in x relative to previous event, used for FPS motion
	int deltax;

	// Change in y relative to previous event, used for FPS motion
	int deltay;

	static const EventType type = EventType::MouseMoved;

	MouseMoveData(unsigned x, unsigned y, unsigned screenx, unsigned screeny,
		int deltax, int deltay);
};

/**
* Data passed with mouse input events
*/
struct MouseInputData
{
	MouseInput button;
	ButtonState state;
	ModifierState modifiers;
	static const EventType type = EventType::MouseInput;

	MouseInputData(MouseInput button, ButtonState state,
		ModifierState modifiers);
};

/**
* Data passed with mouse wheel events
*/
struct MouseWheelData
{
	double delta;
	ModifierState modifiers;
	static const EventType type = EventType::MouseWheel;

	MouseWheelData(double delta, ModifierState modifiers);
};



/**
* Data passed for touch events
*/
struct TouchData
{
	unsigned numTouches;

	TouchPoint touches[32];

	static const EventType type = EventType::Touch;
};

/**
* Data passed for gamepad events
*/
struct GamepadData
{
	// If the gamepad is connected or not
	bool connected;

	// Gamepad Index
	size_t index;

	// String id of the brand of the gamepad
	const char* id;

	// String id that lays out controller mapping (Southpaw, etc.)
	const char* mapping;

	// Analog Axis input data, such as joysticks, normalized range [-1, 1]
	double axis[64];

	// The number of analog axes
	unsigned numAxes;

	// Analog gamepad buttons like triggers, bound to [0, 1].
	double analogButton[64];

	bool digitalButton[64];

	// Number of digital buttons and analog buttons
	unsigned numButtons;

	static const EventType type = EventType::Gamepad;
};

/**
* SDL does something similar:
* <https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlevent.html>
*/
union EventData
{
	FocusData focus;
	ResizeData resize;
	DPIData dpi;
	KeyboardData keyboard;
	MouseMoveData mouseMoved;
	MouseInputData mouseInput;
	MouseWheelData mouseWheel;
	TouchData touch;
	GamepadData gamepad;
	MouseRawData mouseRaw;

	EventData()
	{
	}

	~EventData()
	{
	}
};

class Event
{
public:
	Event(EventType type, Window* window);

	Event(FocusData data, Window* window);

	Event(ResizeData data, Window* window);

	Event(KeyboardData data, Window* window);

	Event(MouseMoveData data, Window* window);

	Event(MouseRawData data, Window* window);

	Event(MouseInputData data, Window* window);

	Event(MouseWheelData data, Window* window);

	Event(TouchData data, Window* window);

	Event(GamepadData data, Window* window);

	~Event();

	// The event's type
	EventType type;

	// Pointer to a CrossWindow window
	Window* window;

	// Inner data of the event
	EventData data;
};

SE_NAMESPACE_WND_END
//=============================================================================