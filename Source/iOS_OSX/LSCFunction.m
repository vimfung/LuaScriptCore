//
//  LSCFunction.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/27.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCFunction.h"
#import "LSCFunction_Private.h"
#import "LSCContext_Private.h"
#import "LSCValue_Private.h"

@implementation LSCFunction

- (instancetype)initWithContext:(LSCContext *)context index:(NSInteger)index
{
    if (self = [super init])
    {
        self.context = context;
        
        lua_State *state = context.state;
        if (index < 0)
        {
            index = lua_gettop(state) + index + 1;
        }
        
        if (lua_istable(state, lua_upvalueindex(0)))
        {
            self.index = [NSUUID UUID].UUIDString;
            
            lua_pushvalue(state, lua_upvalueindex(0));
            
            lua_pushvalue(state, (int)index);
            lua_setfield(state, -2, self.index.UTF8String);
            
            lua_pop(state, 1);
        }
    }
    
    return self;
}

- (void)dealloc
{
    if (self.index)
    {
        //移除索引中的方法
        lua_State *state = self.context.state;
        if (lua_istable(state, lua_upvalueindex(0)))
        {
            lua_pushvalue(state, lua_upvalueindex(0));
            lua_pushnil(state);
            lua_setfield(state, -2, self.index.UTF8String);
            lua_pop(state, 1);
        }
    }
}

- (LSCValue *)invokeWithArguments:(NSArray<LSCValue *> *)arguments
{
    __weak LSCFunction *theFunc = self;
    lua_State *state = self.context.state;
    
    LSCValue *retValue = nil;
    
    if (lua_istable(state, lua_upvalueindex(0)))
    {
        lua_pushvalue(state, lua_upvalueindex(0));
        lua_getfield(state, -1, self.index.UTF8String);
        
        if (lua_isfunction(state, -1))
        {
            [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
                
                [value pushWithContext:theFunc.context];
                
            }];
            
            if (lua_pcall(state, (int)arguments.count, 1, 0) == 0)
            {
                retValue = [LSCValue valueWithContext:self.context atIndex:-1];
            }
            else
            {
                //调用失败
                LSCValue *value = [LSCValue valueWithContext:self.context atIndex:-1];
                NSString *errMessage = [value toString];
                [self.context raiseExceptionWithMessage:errMessage];
            }
        }
        
        lua_pop(state, 2);
    }
    
    if (!retValue)
    {
        retValue = [LSCValue nilValue];
    }
    
    //释放内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return retValue;
}

- (void)push
{
    lua_State *state = self.context.state;
    if (lua_istable(state, lua_upvalueindex(0)))
    {
        lua_getfield(state, lua_upvalueindex(0), self.index.UTF8String);
    }
    else
    {
        lua_pushnil(state);
    }
}

@end
