#pragma once

//=============================================================================
SE_NAMESPACE_BEGIN

typedef void (*TaskFunc)(void *user, uintptr_t arg);

template <class T, void (T::*callback)(size_t)>
static void MemberTaskFunc(void *userData, size_t arg)
{
	T *pThis = static_cast<T*>(userData);
	(pThis->*callback)(arg);
}

template <class T, void (T::*callback)()>
static void MemberTaskFunc0(void *userData, size_t)
{
	T* pThis = static_cast<T*>(userData);
	(pThis->*callback)();
}

struct ThreadSystem;

void InitThreadSystem(ThreadSystem **ppThreadSystem);

void ShutdownThreadSystem(ThreadSystem *threadSystem);

void AddThreadSystemRangeTask(ThreadSystem *threadSystem, TaskFunc task, void *user, uintptr_t count);
void AddThreadSystemRangeTask(ThreadSystem *threadSystem, TaskFunc task, void *user, uintptr_t start, uintptr_t end);
void AddThreadSystemTask(ThreadSystem *threadSystem, TaskFunc task, void *user, uintptr_t index = 0);

bool AssistThreadSystem(ThreadSystem *threadSystem);

bool IsThreadSystemIdle(ThreadSystem *threadSystem);
void WaitThreadSystemIdle(ThreadSystem *threadSystem);

SE_NAMESPACE_END
//=============================================================================