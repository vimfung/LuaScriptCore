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
#import "LSCTuple_Private.h"

@implementation LSCFunction

- (instancetype)initWithContext:(LSCContext *)context index:(NSInteger)index
{
    if (self = [super init])
    {
        self.context = context;
        self.linkId = [NSString stringWithFormat:@"%p", self];
        
        //设置Lua对象到_vars_表中
        [self.context.dataExchanger setLubObjectByStackIndex:index objectId:self.linkId];
        //进行引用
        [self.context retainValue:[LSCValue functionValue:self]];
    }
    
    return self;
}

- (void)dealloc
{
    [self.context releaseValue:[LSCValue functionValue:self]];
}

- (LSCValue *)invokeWithArguments:(NSArray<LSCValue *> *)arguments
{
    LSCValue *retValue = nil;
    
    __weak LSCFunction *theFunc = self;
    lua_State *state = self.context.state;
    
    int top = lua_gettop(state);
    [self.context.dataExchanger getLuaObject:self];
    
    if (lua_isfunction(state, -1))
    {
        int returnCount = 0;
        
        [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
            
            [value pushWithContext:theFunc.context];
            
        }];
        
        if (lua_pcall(state, (int)arguments.count, LUA_MULTRET, 0) == 0)
        {
            returnCount = lua_gettop(state) - top;
            if (returnCount > 1)
            {
                LSCTuple *tuple = [[LSCTuple alloc] init];
                for (int i = 1; i <= returnCount; i++)
                {
                    LSCValue *value = [LSCValue valueWithContext:self.context atIndex:top + i];
                    [tuple addReturnValue:[value toObject]];
                }
                retValue = [LSCValue tupleValue:tuple];
            }
            else if (returnCount == 1)
            {
                retValue = [LSCValue valueWithContext:self.context atIndex:-1];
            }
            
        }
        else
        {
            //调用失败
            returnCount = 1;
            LSCValue *value = [LSCValue valueWithContext:self.context atIndex:-1];
            NSString *errMessage = [value toString];
            [self.context raiseExceptionWithMessage:errMessage];
        }
        
        //弹出返回值
        lua_pop(state, returnCount);
    }
    else
    {
        //弹出func
        lua_pop(state, 1);
    }

    if (!retValue)
    {
        retValue = [LSCValue nilValue];
    }
    
    //释放内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return retValue;
}

#pragma mark - LSCManagedObjectProtocol

- (BOOL)pushWithContext:(LSCContext *)context
{
    return YES;
}

@end
