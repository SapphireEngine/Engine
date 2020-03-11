#include "stdafx.h"
#include "Time.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
static int64_t highResTimerFrequency = 0;
//-----------------------------------------------------------------------------
void initTime()
{
#if SE_PLATFORM_WINDOWS
	LARGE_INTEGER frequency;
	if ( QueryPerformanceFrequency(&frequency) )
		highResTimerFrequency = frequency.QuadPart;
	else
		highResTimerFrequency = 1000LL;
#endif
}
//-----------------------------------------------------------------------------
// Make sure timer frequency is initialized before anyone tries to use it
struct StaticTime
{
	StaticTime()
	{
		initTime();
	}
} staticTimeInst;
//-----------------------------------------------------------------------------
int64_t GetUSec()
{
#if SE_PLATFORM_WINDOWS
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * (int64_t)1e6 / GetTimerFrequency();
#endif
}
//-----------------------------------------------------------------------------
int64_t GetTimerFrequency()
{
#if SE_PLATFORM_WINDOWS
	if ( highResTimerFrequency == 0 )
		initTime();
	return highResTimerFrequency;
#endif
}
//-----------------------------------------------------------------------------
uint32_t GetSystemTime()
{
#if SE_PLATFORM_WINDOWS
	return (uint32_t)timeGetTime();
#endif
}
//-----------------------------------------------------------------------------
uint32_t GetTimeSinceStart()
{
	return (uint32_t)time(NULL);
}
//-----------------------------------------------------------------------------
Timer::Timer()
{
	Reset();
}
//-----------------------------------------------------------------------------
unsigned Timer::GetMSec(bool reset)
{
	unsigned currentTime = GetSystemTime();
	unsigned elapsedTime = currentTime - m_startTime;
	if ( reset )
		m_startTime = currentTime;

	return elapsedTime;
}
//-----------------------------------------------------------------------------
void Timer::Reset()
{
	m_startTime = GetSystemTime();
}
//-----------------------------------------------------------------------------
HiresTimer::HiresTimer()
{
	memset(m_history, 0, sizeof(m_history));
	m_historyIndex = 0;
	Reset();
}
//-----------------------------------------------------------------------------
int64_t HiresTimer::GetUSec(bool reset)
{
	int64_t currentTime = SE_NAMESPACE::GetUSec();
	int64_t elapsedTime = currentTime - m_startTime;

	// Correct for possible weirdness with changing internal frequency
	if ( elapsedTime < 0 )
		elapsedTime = 0;

	if ( reset )
		m_startTime = currentTime;

	m_history[m_historyIndex] = elapsedTime;
	m_historyIndex = (m_historyIndex + 1) % LENGTH_OF_HISTORY;

	return elapsedTime;
}
//-----------------------------------------------------------------------------
int64_t HiresTimer::GetUSecAverage()
{
	int64_t elapsedTime = 0;
	for ( uint32_t i = 0; i < LENGTH_OF_HISTORY; ++i )
		elapsedTime += m_history[i];
	elapsedTime /= LENGTH_OF_HISTORY;

	// Correct for overflow
	if ( elapsedTime < 0 )
		elapsedTime = 0;

	return elapsedTime;
}
//-----------------------------------------------------------------------------
float HiresTimer::GetSeconds(bool reset)
{
	return (float)(GetUSec(reset) / 1e6);
}
//-----------------------------------------------------------------------------
float HiresTimer::GetSecondsAverage()
{
	return (float)(GetUSecAverage() / 1e6);
}
//-----------------------------------------------------------------------------
void HiresTimer::Reset()
{
	m_startTime = SE_NAMESPACE::GetUSec();
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================