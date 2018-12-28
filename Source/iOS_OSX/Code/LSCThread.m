//
//  LSCThread.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/12/24.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCThread.h"
#import "LSCEngineAdapter.h"
#import "LSCContext_Private.h"
#import "LSCSession_Private.h"
#import "LSCValue_Private.h"
#import "LSCTuple.h"

@interface LSCThread ()

/**
 关联ID
 */
@property (nonatomic, copy) NSString *linkId;

@end

@implementation LSCThread

- (instancetype)initWithContext:(LSCContext *)context
                        handler:(LSCFunctionHandler)handler
{
    self = [super initWithCreateStateHandler:^lua_State *{
       
        return [LSCEngineAdapter newThread:context.currentSession.state];
        
    }];
    
    if (self)
    {
        _context = context;
        _finished = NO;
        
        self.linkId = [NSString stringWithFormat:@"%p", self];
        
        int top = [LSCEngineAdapter getTop:_context.currentSession.state];
        //设置Lua对象到_vars_表中
        [self.context.dataExchanger setLubObjectByStackIndex:top objectId:self.linkId];
        //进行引用
        [self.context.dataExchanger retainLuaObject:self];
        
        //将线程状态出栈，让原生层生命周期进行控制
        [LSCEngineAdapter pop:_context.currentSession.state count:1];
        
        //注册调用方法
        [self registerMethodWithName:@"_handler" block:handler];
    }
    
    return self;
}

- (void)dealloc
{
    [self.context.dataExchanger releaseLuaObject:self];
}

- (void)resumeWithArguments:(NSArray<LSCValue *> *)arguments
{
    [self getGlobalForName:@"_handler"];
    
    __weak typeof(self) theThread = self;
    [arguments enumerateObjectsUsingBlock:^(LSCValue * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
    
        [obj pushWithContext:theThread];
        
    }];
    
    int status = [LSCEngineAdapter resumeThread:self.currentSession.state fromThreadState:nil argCount:(int)arguments.count];
    _finished = status != LUA_YIELD;
}

- (void)yieldWithResult:(LSCValue *)result
{
    NSInteger resultCount = 1;
    if (result.valueType == LSCValueTypeTuple)
    {
        resultCount = [result toTuple].count;
    }
    
    [result pushWithContext:self];
    [LSCEngineAdapter yielyThread:self.currentSession.state resultCount:(int)resultCount];
}

@end
