//
//  LSCClass_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCClass.h"
#import "lua.h"

@interface LSCClass ()

/**
 *  类名称
 */
@property (nonatomic, copy) NSString *name;

/**
 *  基类
 */
@property (nonatomic, strong) LSCClass *baseClass;

/**
 *  方法处理器集合
 */
@property(nonatomic, strong) NSMutableDictionary *methodBlocks;

/**
 *  创建实例事件处理器
 */
@property (nonatomic, copy) void (^createHandler)(LSCClassInstance *instance, NSArray *arguments);

/**
 *  销毁实例事件处理器
 */
@property (nonatomic, copy) void (^destoryHandler)(LSCClassInstance *instance);

/**
 *  注册事件处理器
 */
@property (nonatomic, copy) void (^registerHandler) (lua_State *state);

@end
