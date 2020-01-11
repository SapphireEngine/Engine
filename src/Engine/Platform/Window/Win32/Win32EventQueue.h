#pragma once

#include "Platform/Window/Common/Event.h"

//=============================================================================
SE_NAMESPACE_WND_BEGIN

class Window;

class EventQueue
{
public:
	void Update();

	const Event& Front();

	void Pop();

	bool Empty();

	size_t Size();

	LRESULT PushEvent(MSG msg, Window *window);

protected:
	bool m_initialized = false;

	unsigned m_prevMouseX;
	unsigned m_prevMouseY;

	std::queue<Event> m_queue;

	/**
	* Virtual Key Codes in Win32 are an unsigned char:
	* https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
	*/
	typedef Key Win32VKeyToDigitalInputMap[1 << (8 * sizeof(unsigned char))];

	Win32VKeyToDigitalInputMap m_VKeyToDigitalInputMap = 
	{
		/*NONE 0x00*/ Key::KeysMax,
		/*VK_LBUTTON 0x01*/ Key::KeysMax,
		/*VK_RBUTTON 0x02*/ Key::KeysMax,
		/*VK_CANCEL 0x03*/ Key::KeysMax 
	};
};

SE_NAMESPACE_WND_END
//=============================================================================