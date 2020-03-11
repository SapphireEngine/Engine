#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

// High res timer functions
int64_t GetUSec();
int64_t GetTimerFrequency();

// Time related functions
uint32_t GetSystemTime();
uint32_t GetTimeSinceStart();

/// Low res OS timer
class Timer
{
public:
	Timer();
	uint32_t GetMSec(bool reset);
	void Reset();

private:
	uint32_t m_startTime;
};

/// High-resolution OS timer
class HiresTimer
{
public:
	HiresTimer();

	int64_t GetUSec(bool reset);
	int64_t GetUSecAverage();
	float GetSeconds(bool reset);
	float GetSecondsAverage();
	void Reset();

private:
	int64_t m_startTime;

	static const uint32_t LENGTH_OF_HISTORY = 60;
	int64_t m_history[LENGTH_OF_HISTORY];
	uint32_t m_historyIndex;
};

SE_NAMESPACE_END
//=============================================================================