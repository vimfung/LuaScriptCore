//
//  LSCApiAdapter.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LSCValueType.h"
#import "LSCException.h"
#import "lua.h"

@class LSCApiAdapter;
@class LSCContext;
@class LSCCoroutine;
@class LSCState;
@class LSCMainState;
@class LSCTypeDescription;
@class LSCFunctionValue;

/**
 Userdata引用
 */
typedef struct
{
    void *value;
    
} *LSCUserdataRef;

/**
 GC类型
 
 - LSCGCTypeStop: 停止
 - LSCGCTypeRestart: 重启
 - LSCGCTypeCollect: 回收
 - LSCGCTypeCount: 计数
 - LSCGCTypeCountB:
 - LSCGCTypeStep:
 - LSCGCTypeStepPause:
 - LSCGCTypeStepMul:
 */
typedef NS_ENUM(NSUInteger, LSCGCType) {
    LSCGCTypeStop = LUA_GCSTOP,
    LSCGCTypeRestart = LUA_GCRESTART,
    LSCGCTypeCollect = LUA_GCCOLLECT,
    LSCGCTypeCount = LUA_GCCOUNT,
    LSCGCTypeCountB = LUA_GCCOUNTB,
    LSCGCTypeStep = LUA_GCSTEP,
    LSCGCTypeStepPause = LUA_GCSETPAUSE,
    LSCGCTypeStepMul = LUA_GCSETSTEPMUL,
};


/**
 Lua对象类型

 - LSCBasicTypeNone: 无
 - LSCBasicTypeNil: 空类型
 - LSCBasicTypeBoolean: 布尔类型
 - LSCBasicTypeLightUserdata: 轻量用户自定义类型
 - LSCBasicTypeNumber: 数值类型
 - LSCBasicTypeString: 字符串类型
 - LSCBasicTypeTable: 表格类型
 - LSCBasicTypeFunction: 方法类型
 - LSCBasicTypeUserdata: 用户自定义类型
 - LSCBasicTypeThread: 线程类型
 */
typedef NS_ENUM(NSUInteger, LSCBasicType) {
    LSCBasicTypeNone = LUA_TNONE,
    LSCBasicTypeNil = LUA_TNIL,
    LSCBasicTypeBoolean = LUA_TBOOLEAN,
    LSCBasicTypeLightUserdata = LUA_TLIGHTUSERDATA,
    LSCBasicTypeNumber = LUA_TNUMBER,
    LSCBasicTypeString = LUA_TSTRING,
    LSCBasicTypeTable = LUA_TTABLE,
    LSCBasicTypeFunction = LUA_TFUNCTION,
    LSCBasicTypeUserdata = LUA_TUSERDATA,
    LSCBasicTypeThread = LUA_TTHREAD,
};


/**
 观察事件

 - LSCWatchEventCall: 调用时触发
 - LSCWatchEventReturn: 返回前触发
 - LSCWatchEventLine: 每行触发
 - LSCWatchEventCount: 计数触发
 */
typedef NS_ENUM(NSUInteger, LSCWatchEvents) {
    LSCWatchEventCall = LUA_MASKCALL,
    LSCWatchEventReturn = LUA_MASKRET,
    LSCWatchEventLine = LUA_MASKLINE,
    LSCWatchEventCount = LUA_MASKCOUNT,
};

/**
 设置Lua对象选项

 - LSCSetLuaObjectOptionAuto: 弱引用，当lua中释放后该对象将无法访问
 - LSCSetLuaObjectOptionRetain: 引用对象
 - LSCSetLuaObjectOptionRelease: 释放对象
 */
typedef NS_ENUM(NSUInteger, LSCSetLuaObjectOption) {
    LSCSetLuaObjectOptionWeak = 0,
    LSCSetLuaObjectOptionRetain = 1,
    LSCSetLuaObjectOptionRelease = 2,
};


/**
 Lua闭包处理器

 @param apiAdapter 接口适配器
 @param context 上下文对象
 @param mainState 主状态
 @param curState 当前状态
 */
typedef void (^LSCClosureHandler)(LSCApiAdapter *apiAdapter,
                                  LSCContext *context,
                                  LSCMainState *mainState,
                                  LSCState *curState);

NS_ASSUME_NONNULL_BEGIN

/**
 接口适配器，所有上层调用不允许直接调用C层Lua Api，必须使用该类型进行底层访问
 */
@interface LSCApiAdapter : NSObject

/**
 注册Api适配器，对于需要扩展适配器功能接口时需要调用此方法来替换默认适配器

 @param adapterClass 适配器类型
 */
+ (void)registerApiAdapterClass:(Class)adapterClass;

/**
 获取默认Api适配器

 @return 适配器对象
 */
+ (LSCApiAdapter *)defaultApiAdapter;


/**
 处理原生

 @param rawState 状态
 @param block 处理器
 */
+ (void)handleClosureWithRawState:(lua_State *)rawState
                            block:(LSCClosureHandler)block;

/**
 创建状态
 
 @return 状态对象
 */
- (lua_State *)createState;

/**
 获取主状态

 @param state 状态对象
 @return 上下文对象
 */
- (LSCMainState *)getMainStateWithState:(LSCState *)state;

/**
 销毁并关闭上下文

 @param context 上下文对象
 */
- (void)closeContext:(LSCContext *)context;

/**
 添加搜索路径

 @param path 路径
 @param context 状态对象
 */
- (void)addSearchPath:(NSString *)path
              context:(LSCContext *)context;

/**
 注册异常处理方法

 @param errFunc 异常处理方法
 @param context 上下文对象
 */
- (void)registerErrorFunction:(LSCFunctionValue *)errFunc
                      context:(LSCContext *)context;

/**
 获取Lua对象标识

 @param stackIndex 栈索引
 @param context 上下文对象
 
 @return 对象标识, 返回nil则表示对象为空
 */
- (NSString *)getLuaObjectIdWithStackIndex:(int)stackIndex
                                   context:(LSCContext *)context;

/**
 设置Lua对象

 @param objectId 对象标识
 @param option 选项
 @param context 上下文
 */
- (void)setLuaObjectWithId:(NSString *)objectId
                    option:(LSCSetLuaObjectOption)option
                   context:(LSCContext *)context;

/**
 获取栈中数据的类型

 @param stackIndex 栈索引
 @param context 上下文对象
 @return 类型
 */
- (LSCBasicType)getTypeWithStackIndex:(int)stackIndex
                              context:(LSCContext *)context;


/**
 获取字符串类型数据

 @param stackIndex 栈索引
 @param context 上下文对象
 @return 字符串类型数据
 */
- (NSData *)getDataWithStackIndex:(int)stackIndex
                          context:(LSCContext *)context;


/**
 获取数值类型数据

 @param stackIndex 栈索引
 @param context 上下文对象
 @return 数值类型数据
 */
- (NSNumber *)getNumberWithStackIndex:(int)stackIndex
                              context:(LSCContext *)context;

/**
 获取布尔值数据

 @param stackIndex 栈索引
 @param context 上下文对象
 @return 布尔值数据
 */
- (NSNumber *)getBooleanWithStackIndex:(int)stackIndex
                               context:(LSCContext *)context;


/**
 获取Table数据

 @param stackIndex 栈索引
 @param context 上下文对象
 @return Table数据，可能为NSArray或者NSDictionary
 */
- (id)getTableWithStackIndex:(int)stackIndex
                     context:(LSCContext *)context;

/**
 获取对象

 @param stackIndex 栈索引
 @param context 上下文对象
 @return 对象
 */
- (id)getObjectWithStackIndex:(int)stackIndex
                      context:(LSCContext *)context;


/**
 获取upvalue指定索引的值

 @param index 索引
 @param context 上下文对象
 @return 值
 */
- (id<LSCValueType>)getUpvalueWithIndex:(int)index
                                context:(LSCContext *)context;

/**
 获取方法的参数列表

 @param index 起始索引
 @param length 长度
 @param context 上下文
 @return 参数列表
 */
- (NSArray<id<LSCValueType>> *)getArgumentsWithIndex:(int)index
                                              length:(int)length
                                             context:(LSCContext *)context;


/**
 入栈一个nil

 @param context 上下文对象
 */
- (void)pushNilWithContext:(LSCContext *)context;

/**
 入栈一个二进制数组

 @param data 二进制数组
 @param context 上下文对象
 */
- (void)pushData:(NSData *)data context:(LSCContext *)context;

/**
 入栈一个数值

 @param number 数值
 @param context 上下文对象
 */
- (void)pushNumber:(NSNumber *)number context:(LSCContext *)context;

/**
 入栈一个布尔值

 @param boolean 布尔值
 @param context 上下文对象
 */
- (void)pushBoolean:(NSNumber *)boolean context:(LSCContext *)context;

/**
 入栈一个数组

 @param array 数组
 @param context 上下文对象
 */
- (void)pushArray:(NSArray *)array context:(LSCContext *)context;

/**
 入栈一个字典

 @param dictionary 字典
 @param context 上下文对象
 */
- (void)pushDictionary:(NSDictionary *)dictionary context:(LSCContext *)context;


/**
 入栈一个方法

 @param function 方法
 @param context 上下文对象
 */
- (void)pushFunction:(LSCFunctionValue *)function context:(LSCContext *)context;

/**
 入栈一个未知对象

 @param object 对象
 @param context 上下文对象
 */
- (void)pushUnknowObject:(id)object
                 context:(LSCContext *)context;


/**
 入栈一个Lua对象

 @param objectId 对象标识
 @param context 上下文对象
 */
- (void)pushLuaObjectWithId:(NSString *)objectId
                    context:(LSCContext *)context;


/**
 设置异常捕获方法

 @param context 上下文
 @return 异常捕获方法所在栈索引
 */
- (int)setExceptionHandlerWithContext:(LSCContext *)context;

/**
 移除异常捕获方法

 @param index 栈索引
 @param context 上下文
 */
- (void)removeExceptionHandlerWithIndex:(int)index
                                context:(LSCContext *)context;

/**
 执行脚本

 @param script 脚本文本
 @param context 上下文对象
 
 @return 脚本
 */
- (id<LSCValueType>)evalScriptWithString:(NSString *)script
                                 context:(LSCContext *)context;


/**
 执行脚本

 @param path 脚本路径
 @param context 上下文对象
 @return 脚本
 */
- (id<LSCValueType>)evalScriptWithPath:(NSString *)path
                               context:(LSCContext *)context;

/**
 设置全局变量

 @param value 值
 @param name 名称
 @param context 上下文对象
 */
- (void)setGlobalWithValue:(id<LSCValueType>)value
                      name:(NSString *)name
                   context:(LSCContext *)context;

/**
 获取全局变量

 @param name 名称
 @param context 上下文对象
 @return 值
 */
- (id<LSCValueType>)getGlobalWithName:(NSString *)name
                              context:(LSCContext *)context;


/**
 调用方法

 @param functionId 方法标识
 @param arguments 参数列表
 @param context 上下文对象
 @return 返回值
 */
- (id<LSCValueType>)callFunctionWithId:(NSString *)functionId
                             arguments:(NSArray<id<LSCValueType>> *)arguments
                               context:(LSCContext *)context;

/**
 进行垃圾回收

 @param context 上下文对象
 */
- (void)gcWithContext:(LSCContext *)context;

#pragma mark - Error

/**
 抛出异常
 
 @param message 异常消息
 @param context 上下文对象
 */
- (void)raiseErrorWithMessage:(NSString *)message
                      context:(LSCContext *)context;

/**
 中断执行，注：该操作会调用abort终止操作，请慎用

 @param message 中断消息
 @param context 上下文对象
 */
- (void)interruptWithMessage:(NSString *)message
                     context:(LSCContext *)context;

#pragma mark - Watch

/**
 开始事件观察

 @param event 事件
 @param state 状态
 @param count 计数器，仅用于Count事件
 */
- (void)startWatchEvent:(LSCWatchEvents)event
                  state:(LSCState *)state
                  count:(NSInteger)count;


/**
 结束事件观察

 @param state 状态
 */
- (void)stopWatchEventWithstate:(LSCState *)state;

#pragma mark - Thread

/**
 创建协程
 
 @param threadState 线程状态指针，用于在调用完成后取得状态对象
 @param context 上下文对象
 
 @return 线程Id
 */
- (NSString *)createThread:(lua_State **)threadState context:(LSCContext *)context;

/**
 关闭线程

 @param threadId 线程Id
 @param context 上下文
 */
- (void)closeThreadWithId:(NSString *)threadId context:(LSCContext *)context;

/**
 启动协程
 
 @param coroutine 协程对象
 @param arguments 参数
 */
- (void)resumeWithCoroutine:(LSCCoroutine *)coroutine
                  arguments:(NSArray<id<LSCValueType>> *)arguments;

@end

NS_ASSUME_NONNULL_END
