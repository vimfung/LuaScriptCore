//
//  LSCClass.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCTypeDefinied.h"

@class LSCClassInstance;

/**
 *  Lua类
 */
@interface LSCClass : NSObject

/**
 *  类名称
 */
@property (nonatomic, copy, readonly) NSString *name;

/**
 *  基类
 */
@property (nonatomic, strong, readonly) LSCClass *baseClass;

/**
 *  获取对象类型
 *
 *  @return 对象类型
 */
+ (LSCClass *)objectClass;

/**
 *  初始化类型
 *
 *  @param name 类型名称, 必须唯一并且不能于Module名称冲突，否则导致注册类型失败
 *  @param baseClass 基类, 为nil则继承于objectClass
 *
 *  @return 类型对象
 */
- (instancetype)initWithName:(NSString *)name
                   baseClass:(LSCClass *)baseClass;

/**
 *  注册实例方法
 *
 *  @param methodName 方法名称，方法名称不能是create或者destory
 *  @param block      方法处理器
 */
- (void)registerInstanceMethodWithName:(NSString *)methodName
                                 block:(LSCValue* (^) (LSCClassInstance *instance, NSArray *arguments))block;

/**
 *  构造实例时触发该事件
 *
 *  @param handler 事件处理器
 */
- (void)onCreate:(void (^)(LSCClassInstance *instance, NSArray *arguments))handler;

/**
 *  销毁实例时触发该事件
 *
 *  @param handler 事件处理器
 */
- (void)onDestory:(void (^)(LSCClassInstance *instance))handler;

@end
