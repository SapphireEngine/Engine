#pragma once

//=============================================================================
SE_NAMESPACE_WND_BEGIN

/**
* Gamepad Button pressed enum
*/
enum class GamepadButton : size_t
{
	DPadUp = 0,
	DPadDown,
	DPadLeft,
	DPadRight,
	StartButton,
	BackButton,
	LThumbClick,
	RThumbClick,
	LShoulder,
	RShoulder,
	AButton,
	BButton,
	XButton,
	YButton,
	GamepadButtonMax
};

/**
* Gamepad analog stick enum
*/
enum class AnalogInput : size_t
{
	// gamepad
	AnalogLeftTrigger,
	AnalogRightTrigger,
	AnalogLeftStickX,
	AnalogLeftStickY,
	AnalogRightStickX,
	AnalogRightStickY,

	// mouse
	AnalogMouseX,
	AnalogMouseY,
	AnalogMouseScroll,

	AnalogInputsMax
};

typedef const char*
AnalogToStringMap[static_cast<size_t>(AnalogInput::AnalogInputsMax)];

SE_NAMESPACE_WND_END
//=============================================================================