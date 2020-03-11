#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

#if SE_PLATFORM_WINDOWS
typedef unsigned ThreadID;
#else
#define ThreadID pthread_t
#endif

#define TIMEOUT_INFINITE UINT32_MAX

/// Operating system mutual exclusion primitive.
struct Mutex
{
	static const uint32_t DefaultSpinCount = 1500;

	bool Init(uint32_t spinCount = DefaultSpinCount, const char *name = NULL);
	void Destroy();

	void Acquire();
	bool TryAcquire();
	void Release();

#if SE_PLATFORM_WINDOWS
	CRITICAL_SECTION handle;
#else
	pthread_mutex_t handle;
	uint32_t spinCount;
#endif
};

struct MutexLock
{
	MutexLock(Mutex &rhs) : mutex(rhs)
	{
		rhs.Acquire();
	}
	~MutexLock()
	{
		mutex.Release();
	}

	MutexLock(const MutexLock&) = delete;
	MutexLock& operator=(const MutexLock&) = delete;

	Mutex &mutex;
};

struct ConditionVariable
{
	bool Init(const char *name = NULL);
	void Destroy();

	void Wait(const Mutex &mutex, uint32_t md = TIMEOUT_INFINITE);
	void WakeOne();
	void WakeAll();

#if SE_PLATFORM_WINDOWS
	void *handle;
#else
	pthread_cond_t handle;
#endif
};

typedef void(*ThreadFunction)(void*);

/// Work queue item.
struct ThreadDesc
{
	/// Work item description and thread index (Main thread => 0)
	ThreadFunction func;
	void *data;
};

#if SE_PLATFORM_WINDOWS
typedef void *ThreadHandle;
#endif

ThreadHandle CreateThread(ThreadDesc *pItem);
void DestroyThread(ThreadHandle handle);
void JoinThread(ThreadHandle handle);

struct Thread
{
	static ThreadID mainThreadID;
	static void SetMainThread();
	static ThreadID GetCurrentThreadID();
	static void GetCurrentThreadName(char *buffer, int buffer_size);
	static void SetCurrentThreadName(const char *name);
	static bool IsMainThread();
	static void Sleep(unsigned mSec);
	static unsigned int GetNumCPUCores();
};

// Max thread name should be 15 + null character
#ifndef MAX_THREAD_NAME_LENGTH
#	define MAX_THREAD_NAME_LENGTH 31
#endif

#if SE_PLATFORM_WINDOWS
void Sleep(unsigned sec);
#endif

SE_NAMESPACE_END
//=============================================================================