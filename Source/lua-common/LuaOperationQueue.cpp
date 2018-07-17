//
// Created by 冯鸿杰 on 2018/7/2.
//

#include <LuaDefine.h>
#include "LuaOperationQueue.h"



using namespace cn::vimfung::luascriptcore;

void LuaOperationQueue::performAction(std::function<void(void)> const& action)
{
    std::thread::id tid = std::this_thread::get_id();

    if (_curThreadId != tid)
    {
        //非同一线程需要进行锁定
        _threadLocker.lock();
        _curThreadId = tid;
        action();
        _threadLocker.unlock();
    }
    else
    {
        //同一线程无需锁定
        action();
    }
}