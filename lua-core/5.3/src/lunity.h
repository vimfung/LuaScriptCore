//
//  lunity.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/16.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#ifndef lunity_h
#define lunity_h

#include <stdio.h>

#if _WINDOWS

#define LuaScriptCoreApi __declspec(dllexport)

#else

#define LuaScriptCoreApi

#endif

#if defined (__cplusplus)
extern "C" {
#endif
    
	typedef void (*UnityDebugLogPtr)(const char *);
    
    /**
     设置Unity的Debug.Log方法
     
     @param fp 方法指针
     */
	LuaScriptCoreApi extern void setUnityDebugLog(UnityDebugLogPtr fp);
    
    /**
     输出日志到Unity中
     
     @param format 消息格式
     @param ... 参数列表
     */
	LuaScriptCoreApi extern void unityDebug(const char *format, ...);
    
#if defined (__cplusplus)
}
#endif

#endif /* lunity_h */
