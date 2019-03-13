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
        [self.context.dataExchanger retainLuaObject:self];
    }
    
    return self;
}

- (void)dealloc
{
    [self.context.dataExchanger releaseLuaObject:self];
}

- (LSCValue *)invokeWithArguments:(NSArray<LSCValue *> *)arguments
{
    return [self invokeWithArguments:arguments scriptController:nil];
}

- (LSCValue *)invokeWithArguments:(NSArray<LSCValue *> *)arguments
                 scriptController:(LSCScriptController *)scriptController
{
    __block LSCValue *retValue = nil;
    [self.context.optQueue performAction:^{
        
        LSCSession *session = self.context.currentSession;
        lua_State *state = session.state;
        
        session.scriptController = scriptController;
        
        int errFuncIndex = [self.context catchLuaExceptionWithState:state
                                                              queue:self.context.optQueue];
        int top = [LSCEngineAdapter getTop:state];
        [self.context.dataExchanger getLuaObject:self];
        
        if ([LSCEngineAdapter isFunction:state index:-1])
        {
            int returnCount = 0;
            
            [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
                
                [value pushWithContext:self.context];
                
            }];
            
            if ([LSCEngineAdapter pCall:state nargs:(int)arguments.count nresults:LUA_MULTRET errfunc:errFuncIndex] == 0)
            {
                returnCount = [LSCEngineAdapter getTop:state] - top;
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
                returnCount = [LSCEngineAdapter getTop:state] - top;
            }
            
            //弹出返回值
            [LSCEngineAdapter pop:state count:returnCount];
        }
        else
        {
            //弹出func
            [LSCEngineAdapter pop:state count:1];
        }
        
        //移除异常捕获方法
        [LSCEngineAdapter remove:state index:errFuncIndex];
        
        if (!retValue)
        {
            retValue = [LSCValue nilValue];
        }
        
        //释放内存
        [self.context gc];
        
        session.scriptController = nil;
        
    }];
    
    return retValue;
}

#pragma mark - LSCManagedObjectProtocol

- (BOOL)pushWithContext:(LSCContext *)context
{
    //由于function都是从lua层回传到原生层，因此在变量表中一定会有记录，所以这里不需要做任何push操作
    return YES;
}

- (BOOL)pushWithState:(lua_State *)state queue:(LSCOperationQueue *)queue
{
    //由于function都是从lua层回传到原生层，因此在变量表中一定会有记录，所以这里不需要做任何push操作
    return YES;
}

@end
