#pragma once

//=============================================================================
SE_NAMESPACE_WND_BEGIN

// The state of a button press, be it keyboard, mouse, etc.
enum ButtonState : size_t
{
	Pressed = 0,
	Released,
	ButtonStateMax
};

// The state of modifier keys such as ctrl, alt, shift, and the windows/command buttons. Pressed is true, released is false;
struct ModifierState
{
	// Ctrl key
	bool ctrl;

	// Alt key
	bool alt;

	// Shift key
	bool shift;

	// Meta buttons such as the Windows button or Mac's Command button
	bool meta;

	ModifierState(bool ctrl = false, bool alt = false, bool shift = false,
		bool meta = false);
};

// Key event enum
enum class Key : size_t
{
	// Keyboard
	Escape = 0,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,
	Num0,
	Minus,
	Equals,
	Back,
	Tab,
	Q,
	W,
	E,
	R,
	T,
	Y,
	U,
	I,
	O,
	P,
	LBracket,
	RBracket,
	Enter,
	LControl,
	A,
	S,
	D,
	F,
	G,
	H,
	J,
	K,
	L,
	Semicolon,
	Apostrophe,
	Grave,
	LShift,
	Backslash,
	Z,
	X,
	C,
	V,
	B,
	N,
	M,
	Comma,
	Period,
	Slash,
	RShift,
	Multiply,
	LAlt,
	Space,
	Capital,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	Numlock,
	Scroll,
	Numpad7,
	Numpad8,
	Numpad9,
	Subtract,
	Numpad4,
	Numpad5,
	Numpad6,
	Add,
	Numpad1,
	Numpad2,
	Numpad3,
	Numpad0,
	Decimal,
	F11,
	F12,
	Numpadenter,
	RControl,
	Divide,
	sysrq,
	RAlt,
	Pause,
	Home,
	Up,
	PgUp,
	Left,
	Right,
	End,
	Down,
	PgDn,
	Insert,
	Del,
	LWin,
	RWin,
	Apps,

	KeysMax
};

typedef const char* KeyToCharMap[static_cast<size_t>(Key::KeysMax)];
typedef Key CharToKeyMap[static_cast<size_t>(Key::KeysMax)];

/**
* A map of the Keys enum to chars for matching keyboard event data.
* Convenient for converting Key(s) to strings for serialization
*/
static KeyToCharMap sKeyToCharMap;

/**
* Converts a key to a string for serialization
*/
const char* convertKeyToString(Key key);

/**
* A map of strings to Keys for matching keyboard event data.
* Useful for deserialization of strings to Keys
*/
static CharToKeyMap sCharToKeyMap;

/**
* Converts a string name to a Key for deserialization
*/
Key convertStringToKey(const char* str);

SE_NAMESPACE_WND_END
//=============================================================================