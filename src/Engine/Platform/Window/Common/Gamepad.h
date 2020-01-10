#pragma once

//=============================================================================
SE_NAMESPACE_WND_BEGIN

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

SE_NAMESPACE_WND_END
//=============================================================================