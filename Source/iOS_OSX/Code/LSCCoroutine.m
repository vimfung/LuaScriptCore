//
//  LSCThread.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/12/24.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#import "LSCCoroutine.h"
#import "LSCCoroutine+Private.h"
#import "LSCContext_Private.h"
#import "LSCSession_Private.h"
#import "LSCValue_Private.h"
#import "LSCTuple.h"

@implementation LSCCoroutine

- (instancetype)initWithContext:(LSCContext *)context
{
    if (self)
    {
        _context = context;
        self.linkId = [NSString stringWithFormat:@"%p", self];
        
        __weak typeof(self) theCoroutine = self;
        [self.context.optQueue performAction:^{
            
            theCoroutine.state = [LSCEngineAdapter newThread:context.currentSession.state];
            
            int top = [LSCEngineAdapter getTop:context.currentSession.state];
            
            //设置Lua对象到_vars_表中
            [context.dataExchanger setLubObjectByStackIndex:top objectId:theCoroutine.linkId];
            //进行引用
            [context.dataExchanger retainLuaObject:theCoroutine];
            
            //将线程状态出栈，让原生层生命周期进行控制
            [LSCEngineAdapter pop:context.currentSession.state count:1];
            
        }];
    }
    
    return self;
}

- (void)dealloc
{
    [self.context.dataExchanger releaseLuaObject:self];
}

@end
