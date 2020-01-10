#pragma once

#include "Platform/Window/Common/Key.h"
#include "Platform/Window/Common/Mouse.h"
#include "Platform/Window/Common/Gamepad.h"

//=============================================================================
SE_NAMESPACE_WND_BEGIN

class Window;

// The state of a button press, be it keyboard, mouse, etc.
enum ButtonState : size_t
{
	Pressed = 0,
	Released,
	ButtonStateMax
};

enum class EventType : size_t
{
	None = 0,

	Close,		// Closing a window	
	Create,		// Creating a window	
	Focus,		// Focus/Unfocus on a window	
	Paint,		// Paint events, a good time to present any graphical changes	
	Resize,		// Resizing a window	
	DPI,		// Change in the screen DPI scaling (such as moving a window to a monitor with a larger DPI.	
	Keyboard,	// Keyboard input such as press/release events	
	MouseMoved,	// Mouse moving events	
	MouseRaw,	// Raw mouse data events	
	MouseWheel,	// Mouse scrolling events	
	MouseInput,	// Mouse button press events	
	Touch,		// Touch events	
	Gamepad,	// Gamepad Input Events such as analog sticks, button presses	
	DropFile,	// Dropping a file on the window	
	HoverFile,	// Hovering a file over a window

	EventTypeMax
};

// Focus data passed with Focus events
struct FocusData
{
	FocusData(bool focused) : focused(focused) {};
	
	bool focused;	// true if focused, false if not
};

// Resize data passed with Resize events
struct ResizeData
{
	ResizeData(unsigned width, unsigned height, bool resizing) : width(width), height(height), resizing(resizing){}
		
	unsigned width;		// new width of window viewport	
	unsigned height;	// New height of window viewport	
	bool resizing;		// In the process of resizing
};

// DPI data passed with DPI events
struct DPIData
{
	float scale;
};

// Data sent during keyboard events
struct KeyboardData
{
	KeyboardData(Key key, ButtonState state, ModifierState modifiers) : key(key), state(state), modifiers(modifiers){}

	Key key;
	ButtonState state;
	ModifierState modifiers;
};

struct MouseRawData
{
	MouseRawData(int deltax, int deltay) : deltax(deltax), deltay(deltay) {}

	int deltax;
	int deltay;
};

// The event data passed with mouse events click, mouse moving events
struct MouseMoveData
{	
	MouseMoveData(unsigned x, unsigned y, unsigned screenx, unsigned screeny, int deltax, int deltay) : x(x), y(y), screenx(screenx), screeny(screeny), deltax(deltax), deltay(deltay) { }

	unsigned x;			// Current x position relative to active window	
	unsigned y;			// Current y position relative to active window	
	unsigned screenx;	// Current global x position	
	unsigned screeny;	// Current in global y position	
	int deltax;			// Change in x relative to previous event, used for FPS motion	
	int deltay;			// Change in y relative to previous event, used for FPS motion
};

// Data passed with mouse input events
struct MouseInputData
{
	MouseInputData(MouseInput button, ButtonState state, ModifierState modifiers) : button(button), state(state), modifiers(modifiers) {}

	MouseInput button;
	ButtonState state;
	ModifierState modifiers;	
};

// Data passed with mouse wheel events
struct MouseWheelData
{
	MouseWheelData(double delta, ModifierState modifiers) : delta(delta), modifiers(modifiers) { }

	double delta;
	ModifierState modifiers;	
};

// Data passed for touch events
struct TouchData
{
	unsigned numTouches;
	TouchPoint touches[32];
};

// Data passed for gamepad events
struct GamepadData
{	
	bool connected;			// If the gamepad is connected or not	
	size_t index;			// Gamepad Index	
	const char* id;			// String id of the brand of the gamepad	
	const char* mapping;	// String id that lays out controller mapping (Southpaw, etc.)	
	double axis[64];		// Analog Axis input data, such as joysticks, normalized range [-1, 1]	
	unsigned numAxes;		// The number of analog axes	
	double analogButton[64];// Analog gamepad buttons like triggers, bound to [0, 1].
	bool digitalButton[64];	// Number of digital buttons and analog buttons	
	unsigned numButtons;
};

union EventData
{
	EventData() {}
	~EventData() {}

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
};

class Event
{
public:
	Event(EventType type, Window *window);
	Event(FocusData data, Window *window);
	Event(ResizeData data, Window *window);
	Event(KeyboardData data, Window *window);
	Event(MouseMoveData data, Window *window);
	Event(MouseRawData data, Window *window);
	Event(MouseInputData data, Window *window);
	Event(MouseWheelData data, Window *window);
	Event(TouchData data, Window *window);
	Event(GamepadData data, Window *window);
	~Event() = default;
	
	EventType type;	// The event's type	
	Window *window;	// Pointer to a window	
	EventData data;	// Inner data of the event
};

SE_NAMESPACE_WND_END
//=============================================================================