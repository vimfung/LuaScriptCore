//
//  lunity.c
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/16.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#include "lunity.h"
#include <stdarg.h>

#if defined (__cplusplus)
extern "C" {
#endif
    
    static UnityDebugLogPtr _unitDebugLogPtr = NULL;
    
    void setUnityDebugLog(UnityDebugLogPtr fp)
    {
        _unitDebugLogPtr = fp;
    }
    
    void unityDebug(const char *format, ...)
    {
        va_list arg_ptr;
        va_start(arg_ptr, format);

        if (_unitDebugLogPtr == NULL)
        {
            vprintf(format, arg_ptr);
            printf("\n");
        }
        else
        {
            char buffer[4096] = {0};
#if _WINDOWS
            vsprintf_s(buffer, sizeof(buffer), format, arg_ptr);
#else
            vsnprintf(buffer, sizeof(buffer), format, arg_ptr);
#endif
            _unitDebugLogPtr (buffer);
        }
        
        va_end(arg_ptr);
    }
    
    
#if defined (__cplusplus)
}
#endif
