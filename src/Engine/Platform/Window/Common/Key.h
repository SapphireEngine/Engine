#pragma once

//=============================================================================
SE_NAMESPACE_WND_BEGIN

// The state of modifier keys such as ctrl, alt, shift, and the windows/command buttons. Pressed is true, released is false;
struct ModifierState
{
	ModifierState() = default;
	ModifierState(bool ctrl, bool alt, bool shift, bool meta) : ctrl(ctrl), alt(alt), shift(shift), meta(meta) {}
	
	bool ctrl = false;	// Ctrl key	
	bool alt = false;	// Alt key	
	bool shift = false;	// Shift key	
	bool meta = false;	// Meta buttons such as the Windows button or Mac's Command button
};

enum class Key : size_t
{
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

SE_NAMESPACE_WND_END
//=============================================================================