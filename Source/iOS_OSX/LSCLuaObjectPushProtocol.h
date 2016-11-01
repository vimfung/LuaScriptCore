//
//  LSCLuaObjectPushProtocol.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/1.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCContext;


/**
 对象入栈协议
 */
@protocol LSCLuaObjectPushProtocol <NSObject>


@required

/**
 入栈数据

 @param context 上下文对象
 */
- (void)pushWithContext:(LSCContext *)context;

@end
