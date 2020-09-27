//
//  LSCContext.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/5.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCContext.h"
#import "LSCApiAdapter.h"
#import "LSCValue.h"
#import "LSCWeakReference.h"
#import "LSCState+Private.h"
#import "LSCFunctionValue.h"
#import "LSCStringValue.h"
#import "LSCCoroutine.h"
#import <pthread.h>
#import "LSCContext+Private.h"
#import "LSCMainState.h"

@implementation LSCContext

- (void)setStateWatcher:(LSCStateWatcher *)stateWatcher
{
    self.mainState.watcher = stateWatcher;
}

- (void)addSearchPath:(NSString *)path
{
    LSCApiAdapter *adapter = [LSCApiAdapter defaultApiAdapter];
    [adapter addSearchPath:path context:self];
}

- (void)onException:(LSCExceptionHandler)handler
{
    if (handler)
    {
        __weak typeof(self) weakContext = self;
        self.exceptionFunction = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {
           
            if (arguments.count > 0)
            {
                NSString *errMsg = nil;
                id<LSCValueType> msgValue = arguments[0];
                if ([msgValue isKindOfClass:[LSCStringValue class]])
                {
                    errMsg = msgValue.rawValue;
                }
                
                handler (weakContext, errMsg);
            }
            
            return nil;
            
        }];
    }
    else
    {
        self.exceptionFunction = nil;
    }
    
    //注册错误处理方法
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter registerErrorFunction:self.exceptionFunction context:self];
}

- (void)raiseExceptionWithMessage:(NSString *)message
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter raiseErrorWithMessage:message context:self];
}

- (id<LSCValueType>)evalScriptFromString:(NSString *)string
{
    LSCApiAdapter *adapter = [LSCApiAdapter defaultApiAdapter];
    return [adapter evalScriptWithString:string context:self];
}

- (id<LSCValueType>)evalScriptFromFile:(NSString *)path
{
    LSCApiAdapter *adapter = [LSCApiAdapter defaultApiAdapter];
    return [adapter evalScriptWithPath:path context:self];
}

- (void)setGlobal:(nullable id<LSCValueType>)value forName:(NSString *)name
{
    LSCApiAdapter *adapter = [LSCApiAdapter defaultApiAdapter];
    [adapter setGlobalWithValue:value name:name context:self];
}

- (id<LSCValueType>)getGlobalForName:(NSString *)name
{
    LSCApiAdapter *adapter = [LSCApiAdapter defaultApiAdapter];
    return [adapter getGlobalWithName:name context:self];
}

- (void)gc
{
    [self _delayedGC];
}

#pragma mark Module Methods

- (void)useModule:(Class<LSCModule>)module
{
    if (!self.modules[[module moduleId]])
    {
        [self.modules setObject:module forKey:[module moduleId]];
        [module registerModuleWithContext:self];
    }
}

- (id)getUserdataWithKey:(NSString *)key
                  module:(Class<LSCModule>)module
{
    NSString *moduleId = [module moduleId];
    if (self.modules[moduleId])
    {
        //模块已登记
        NSMutableDictionary *userdataMap = self.moduleUserdatas[moduleId];
        if (!userdataMap)
        {
            userdataMap = [NSMutableDictionary dictionary];
            [self.moduleUserdatas setObject:userdataMap forKey:moduleId];
        }
        
        return userdataMap[key];
    }
    
    return nil;
}

- (void)setUserdata:(id)userdata
             forKey:(NSString *)key
             module:(Class<LSCModule>)module
{
    NSString *moduleId = [module moduleId];
    if (self.modules[moduleId])
    {
        //模块已登记
        NSMutableDictionary *userdataMap = self.moduleUserdatas[moduleId];
        if (!userdataMap)
        {
            userdataMap = [NSMutableDictionary dictionary];
            [self.moduleUserdatas setObject:userdataMap forKey:moduleId];
        }
        
        [userdataMap setObject:userdata forKey:key];
    }
}

#pragma mark - Rewrite

- (instancetype)init
{

    if (self = [super init])
    {
        self.mainState = [[LSCMainState alloc] initWithContext:self];
        self.modules = [NSMutableDictionary dictionary];
        self.moduleUserdatas = [NSMutableDictionary dictionary];
        
        //设置搜索路径
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        [self addSearchPath:[resourcePath stringByAppendingString:@"/"]];
        
        //设置默认的异常捕获方法
        [self onException:^(LSCContext *context, NSString *message) {
           
            NSLog(@"<LSC_Error> : %@", message);
            
        }];
    }
    
    return self;
}

- (void)dealloc
{
    [[LSCApiAdapter defaultApiAdapter] closeContext:self];
}

#pragma mark - Private

/**
 延迟回收，使用该方法可以在未来的某一个时间点进行回收，
 对于在执行脚本时频繁gc操作可以使用该方法以提高执行效率
 */
- (void)_delayedGC
{
    if (!self.gcFlag)
    {
        //进行定时内存回收检测
        self.gcFlag = YES;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_global_queue(0, 0), ^{
            
            [[LSCApiAdapter defaultApiAdapter] gcWithContext:self];
            
            self.gcFlag = NO;
        });
    }
}

@end
