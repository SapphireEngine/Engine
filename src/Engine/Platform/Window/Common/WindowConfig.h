#pragma once

//=============================================================================
SE_NAMESPACE_WND_BEGIN

struct WindowConfig
{
	// States
	bool visible = false;
	bool fullscreen = false;
	bool resizable = false;
	unsigned monitor = 0;
	// App Data
	std::wstring title = L"Sapphire";
	std::wstring name = L"SapphireApp";
	std::string iconPath = "";
	// Transform
	long x = 0;
	long y = 0;
	bool centered = true;
	unsigned width = 640;
	unsigned height = 360;
};

struct UVec2
{
	UVec2(unsigned x = 0, unsigned y = 0) : x(x), y(y) {}
	unsigned x = 0;
	unsigned y = 0;
};

SE_NAMESPACE_WND_END
//=============================================================================