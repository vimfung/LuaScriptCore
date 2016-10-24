//
//  LSCObjectValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCObjectValue.h"
#import "LSCValue_Private.h"
#import "LSCObjectClass_Private.h"
#import "LSCObjectClass.h"
#import "lauxlib.h"

@implementation LSCObjectValue

+ (LSCValue *)valueWithState:(lua_State *)state atIndex:(NSInteger)index
{
    LSCValue *value = nil;
    
    //先判断是否为类实例对象
    if (lua_type(state, (int)index) == LUA_TUSERDATA)
    {
        void **ref = lua_touserdata(state, (int)index);
        LSCObjectClass *instance = (__bridge LSCObjectClass *)*ref;
        value = [LSCObjectValue objectValue:instance];
    }
    
    if (!value)
    {
        value = [super valueWithState:state atIndex:index];
    }
    
    return value;
}

- (void)pushWithState:(lua_State *)state
{
    BOOL hasProcess = NO;
    if (self.valueType == LSCValueTypeObject)
    {
        id obj = [self toObject];
        if ([obj isKindOfClass:[LSCObjectClass class]])
        {
            void **ref = [LSCObjectClass _findLuaRef:obj];
            if (ref != NULL)
            {
                //直接原指针返回并不等于原始变量，因此需要重新绑定元表
                lua_pushlightuserdata(state, ref);
                luaL_getmetatable(state, [[obj class] moduleName].UTF8String);
                if (lua_istable(state, -1))
                {
                    lua_setmetatable(state, -2);
                }
            }
            else
            {
                lua_pushnil(state);
            }
            
            hasProcess = YES;
        }
    }
    
    if (!hasProcess)
    {
        [super pushWithState:state];
    }
    
}

@end
