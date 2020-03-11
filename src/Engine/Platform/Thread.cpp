#include "stdafx.h"
#include "Thread.h"

//=============================================================================
SE_NAMESPACE_BEGIN
#if SE_PLATFORM_WINDOWS
//-----------------------------------------------------------------------------
bool Mutex::Init(uint32_t spinCount, const char *name)
{
	return InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)&handle, (DWORD)spinCount);
}
//-----------------------------------------------------------------------------
void Mutex::Destroy()
{
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)&handle;
	DeleteCriticalSection(cs);
	handle = {};
}
//-----------------------------------------------------------------------------
void Mutex::Acquire()
{
	EnterCriticalSection((CRITICAL_SECTION*)&handle);
}
//-----------------------------------------------------------------------------
bool Mutex::TryAcquire()
{
	return TryEnterCriticalSection((CRITICAL_SECTION*)&handle);
}
//-----------------------------------------------------------------------------
void Mutex::Release()
{
	LeaveCriticalSection((CRITICAL_SECTION*)&handle);
}
//-----------------------------------------------------------------------------
bool ConditionVariable::Init(const char *name)
{
	handle = (CONDITION_VARIABLE*)conf_calloc(1, sizeof(CONDITION_VARIABLE));
	InitializeConditionVariable((PCONDITION_VARIABLE)handle);
	return true;
}
//-----------------------------------------------------------------------------
void ConditionVariable::Destroy()
{
	conf_free(handle);
}
//-----------------------------------------------------------------------------
void ConditionVariable::Wait(const Mutex &mutex, uint32_t ms)
{
	SleepConditionVariableCS((PCONDITION_VARIABLE)handle, (PCRITICAL_SECTION)&mutex.handle, ms);
}
//-----------------------------------------------------------------------------
void ConditionVariable::WakeOne()
{
	WakeConditionVariable((PCONDITION_VARIABLE)handle);
}
//-----------------------------------------------------------------------------
void ConditionVariable::WakeAll()
{
	WakeAllConditionVariable((PCONDITION_VARIABLE)handle);
}
//-----------------------------------------------------------------------------
ThreadID Thread::mainThreadID;
//-----------------------------------------------------------------------------
void Thread::SetMainThread()
{
	mainThreadID = GetCurrentThreadID();
}
//-----------------------------------------------------------------------------
ThreadID Thread::GetCurrentThreadID()
{
	return GetCurrentThreadId();
}
//-----------------------------------------------------------------------------
char* thread_name()
{
	__declspec(thread) static char name[MAX_THREAD_NAME_LENGTH + 1];
	return name;
}
//-----------------------------------------------------------------------------
void Thread::GetCurrentThreadName(char *buffer, int size)
{
	if ( const char* name = thread_name() )
		snprintf(buffer, (size_t)size, "%s", name);
	else
		buffer[0] = 0;
}
//-----------------------------------------------------------------------------
void Thread::SetCurrentThreadName(const char *name)
{
	strcpy_s(thread_name(), MAX_THREAD_NAME_LENGTH + 1, name);
}
//-----------------------------------------------------------------------------
bool Thread::IsMainThread()
{
	return GetCurrentThreadID() == mainThreadID;
}
//-----------------------------------------------------------------------------
DWORD WINAPI ThreadFunctionStatic(void *data)
{
	ThreadDesc* pDesc = (ThreadDesc*)data;
	pDesc->func(pDesc->data);
	return 0;
}
//-----------------------------------------------------------------------------
void Thread::Sleep(unsigned mSec)
{
	::Sleep(mSec);
}
//-----------------------------------------------------------------------------
// threading class (Static functions)
unsigned int Thread::GetNumCPUCores()
{
	_SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	return systemInfo.dwNumberOfProcessors;
}
//-----------------------------------------------------------------------------
ThreadHandle CreateThread(ThreadDesc *desc)
{
	ThreadHandle handle = ::CreateThread(0, 0, ThreadFunctionStatic, desc, 0, 0);
	__assume(handle != NULL);
	return handle;
}
//-----------------------------------------------------------------------------
void DestroyThread(ThreadHandle handle)
{
	__assume(handle != NULL);
	WaitForSingleObject((HANDLE)handle, INFINITE);
	CloseHandle((HANDLE)handle);
	handle = 0;
}
//-----------------------------------------------------------------------------
void JoinThread(ThreadHandle handle)
{
	WaitForSingleObject((HANDLE)handle, INFINITE);
}
//-----------------------------------------------------------------------------
void Sleep(uint32_t mSec)
{
	::Sleep((DWORD)mSec);
}
//-----------------------------------------------------------------------------
#endif
SE_NAMESPACE_END
//=============================================================================