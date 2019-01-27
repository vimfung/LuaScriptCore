//
//  LSCDataExchanger.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>

@class LSCContext;
@class LSCValue;
@class LSCSession;
@class LSCCoroutine;

/**
 数据交换器，用于Lua->OC或者OC->Lua时数据的转换和内存管理方面的操作
 */
@interface LSCDataExchanger : NSObject

/**
 初始化

 @param context 上下文对象
 @return 数据交换器对象
 */
- (instancetype)initWithContext:(LSCContext *)context;

/**
 从栈中获取获取数据

 @param index 栈索引
 @return 数据
 */
- (LSCValue *)valueByStackIndex:(int)index;

/**
 将对象入栈

 @param object 对象
 */
- (void)pushStackWithObject:(id)object;

/**
 将对象入栈

 @param object 对象
 @param coroutine 协程
 */
- (void)pushStackWithObject:(id)object
                 coroutine:(LSCCoroutine *)coroutine;

/**
 获取Lua对象并入栈

 @param nativeObject 原生对象
 */
- (void)getLuaObject:(id)nativeObject;

/**
 设置Lua对象
 
 @param index Lua对象在栈中索引
 @param objectId 对象标识
 */
- (void)setLubObjectByStackIndex:(NSInteger)index
                        objectId:(NSString *)objectId;


/**
 保留对象对应在Lua中的引用

 @param nativeObject 原生对象
 */
- (void)retainLuaObject:(id)nativeObject;

/**
 释放对象对应在Lua中的引用

 @param nativeObject 原生对象
 */
- (void)releaseLuaObject:(id)nativeObject;



@end
