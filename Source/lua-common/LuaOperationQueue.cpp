//
// Created by 冯鸿杰 on 2018/7/2.
//

#include "LuaOperationQueue.h"
#include "LuaError.h"
#include "LuaEngineAdapter.hpp"
#include "LuaSession.h"


using namespace cn::vimfung::luascriptcore;

LuaOperationQueue::LuaOperationQueue()
{
#if _WINDOWS

	InitializeCriticalSection(&_lock);

#else

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&_lock, &attr);

    pthread_mutexattr_destroy(&attr);

#endif
}

LuaOperationQueue::~LuaOperationQueue()
{
#if _WINDOWS
	DeleteCriticalSection(&_lock);
#else
    pthread_mutex_destroy(&_lock);
#endif
}

void LuaOperationQueue::performAction(std::function<void(void)> const& action)
{
#if _WINDOWS

	EnterCriticalSection(&_lock);
	action();
	LeaveCriticalSection(&_lock);

#else

    pthread_mutex_lock(&_lock);
    action();
    pthread_mutex_unlock(&_lock);

#endif
}
