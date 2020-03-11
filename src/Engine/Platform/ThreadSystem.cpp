#include "stdafx.h"
#include "ThreadSystem.h"
#include "Thread.h"

//=============================================================================
SE_NAMESPACE_BEGIN
//-----------------------------------------------------------------------------
struct ThreadedTask
{
	TaskFunc task;
	void *user;
	uintptr_t start;
	uintptr_t end;
};
//-----------------------------------------------------------------------------
enum
{
	MAX_LOAD_THREADS = 16
};
//-----------------------------------------------------------------------------
struct ThreadSystem
{
	ThreadDesc threadDescs[MAX_LOAD_THREADS];
	ThreadHandle thread[MAX_LOAD_THREADS];
	stl::deque<ThreadedTask> loadQueue;
	ConditionVariable queueCond;
	Mutex queueMutex;
	ConditionVariable idleCond;
	uint32_t numLoaders;
	uint32_t numIdleLoaders;
	volatile bool run;
};
//-----------------------------------------------------------------------------
static void taskThreadFunc(void *threadData)
{
	ThreadSystem *threadSystem = (ThreadSystem*)threadData;
	while ( threadSystem->run )
	{
		threadSystem->queueMutex.Acquire();
		++threadSystem->numIdleLoaders;
		while ( threadSystem->run && threadSystem->loadQueue.empty() )
		{
			threadSystem->idleCond.WakeAll();
			threadSystem->queueCond.Wait(threadSystem->queueMutex);
		}
		--threadSystem->numIdleLoaders;
		if ( !threadSystem->loadQueue.empty() )
		{
			ThreadedTask resourceTask = threadSystem->loadQueue.front();
			if ( resourceTask.start + 1 == resourceTask.end )
				threadSystem->loadQueue.pop_front();
			else
				++threadSystem->loadQueue.front().start;
			threadSystem->queueMutex.Release();
			resourceTask.task(resourceTask.user, resourceTask.start);
		}
		else
		{
			threadSystem->queueMutex.Release();
		}
	}
	threadSystem->queueMutex.Acquire();
	++threadSystem->numIdleLoaders;
	threadSystem->idleCond.WakeAll();
	threadSystem->queueMutex.Release();
}
//-----------------------------------------------------------------------------
void InitThreadSystem(ThreadSystem **ppThreadSystem)
{
	ThreadSystem *threadSystem = conf_new(ThreadSystem);

	uint32_t numThreads = stl::max<uint32_t>(Thread::GetNumCPUCores() - 1, 1);
	uint32_t numLoaders = stl::min<uint32_t>(numThreads, MAX_LOAD_THREADS);

	threadSystem->queueMutex.Init();
	threadSystem->queueCond.Init();
	threadSystem->idleCond.Init();

	threadSystem->run = true;
	threadSystem->numIdleLoaders = 0;

	for ( unsigned i = 0; i < numLoaders; ++i )
	{
		threadSystem->threadDescs[i].func = taskThreadFunc;
		threadSystem->threadDescs[i].data = threadSystem;
		threadSystem->thread[i] = CreateThread(&threadSystem->threadDescs[i]);
	}
	threadSystem->numLoaders = numLoaders;

	*ppThreadSystem = threadSystem;
}
//-----------------------------------------------------------------------------
void ShutdownThreadSystem(ThreadSystem *threadSystem)
{
	threadSystem->queueMutex.Acquire();
	threadSystem->run = false;
	threadSystem->queueMutex.Release();
	threadSystem->queueCond.WakeAll();

	uint32_t numLoaders = threadSystem->numLoaders;
	for ( uint32_t i = 0; i < numLoaders; ++i )
	{
		DestroyThread(threadSystem->thread[i]);
	}

	threadSystem->queueCond.Destroy();
	threadSystem->idleCond.Destroy();
	threadSystem->queueMutex.Destroy();
	conf_delete(threadSystem);
}
//-----------------------------------------------------------------------------
void AddThreadSystemRangeTask(ThreadSystem *threadSystem, TaskFunc task, void *user, uintptr_t count)
{
	threadSystem->queueMutex.Acquire();
	threadSystem->loadQueue.emplace_back(ThreadedTask{ task, user, 0, count });
	threadSystem->queueMutex.Release();
	threadSystem->queueCond.WakeAll();
}
//-----------------------------------------------------------------------------
void AddThreadSystemRangeTask(ThreadSystem *threadSystem, TaskFunc task, void *user, uintptr_t start, uintptr_t end)
{
	threadSystem->queueMutex.Acquire();
	threadSystem->loadQueue.emplace_back(ThreadedTask{ task, user, start, end });
	threadSystem->queueMutex.Release();
	threadSystem->queueCond.WakeAll();
}
//-----------------------------------------------------------------------------
void AddThreadSystemTask(ThreadSystem *threadSystem, TaskFunc task, void *user, uintptr_t index)
{
	threadSystem->queueMutex.Acquire();
	threadSystem->loadQueue.emplace_back(ThreadedTask{ task, user, index, index + 1 });
	threadSystem->queueMutex.Release();
	threadSystem->queueCond.WakeAll();
}
//-----------------------------------------------------------------------------
bool AssistThreadSystem(ThreadSystem *threadSystem)
{
	threadSystem->queueMutex.Acquire();
	if ( !threadSystem->loadQueue.empty() )
	{
		ThreadedTask resourceTask = threadSystem->loadQueue.front();
		if ( resourceTask.start + 1 == resourceTask.end )
			threadSystem->loadQueue.pop_front();
		else
			++threadSystem->loadQueue.front().start;
		threadSystem->queueMutex.Release();
		resourceTask.task(resourceTask.user, resourceTask.start);

		return true;
	}
	else
	{
		threadSystem->queueMutex.Release();
		return false;
	}
}
//-----------------------------------------------------------------------------
bool IsThreadSystemIdle(ThreadSystem *threadSystem)
{
	threadSystem->queueMutex.Acquire();
	bool idle = (threadSystem->loadQueue.empty() && threadSystem->numIdleLoaders == threadSystem->numLoaders) || !threadSystem->run;
	threadSystem->queueMutex.Release();
	return idle;
}
//-----------------------------------------------------------------------------
void WaitThreadSystemIdle(ThreadSystem *threadSystem)
{
	threadSystem->queueMutex.Acquire();
	while ( (!threadSystem->loadQueue.empty() || threadSystem->numIdleLoaders < threadSystem->numLoaders) && threadSystem->run )
		threadSystem->idleCond.Wait(threadSystem->queueMutex);
	threadSystem->queueMutex.Release();
}
//-----------------------------------------------------------------------------
SE_NAMESPACE_END
//=============================================================================