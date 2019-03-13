//
//  LSCEngineAdapter.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/8/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "lua.h"
#import "ldo.h"


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
    LSCGCTypeStop = 0,
    LSCGCTypeRestart = 1,
    LSCGCTypeCollect = 2,
    LSCGCTypeCount = 3,
    LSCGCTypeCountB = 4,
    LSCGCTypeStep = 5,
    LSCGCTypeStepPause = 6,
    LSCGCTypeStepMul = 7,
};

/**
 引擎适配器，用于应对不同版本的lua需要做的操作
 */
@interface LSCEngineAdapter : NSObject

/**
 创建新的状态对象

 @return 状态对象
 */
+ (lua_State *)newState;

/**
 关闭状态

 @param state 状态对象
 */
+ (void)close:(lua_State *)state;

/**
 回收内存

 @param state 状态
 @param what 操作
 @param data 数据
 @return 执行结果
 */
+ (int)gc:(lua_State *)state what:(LSCGCType)what data:(int)data;

/**
 打开标准库

 @param state 状态
 */
+ (void)openLibs:(lua_State *)state;

/**
 设置栈顶对象为全局变量

 @param state 状态
 @param name 名称
 */
+ (void)setGlobal:(lua_State *)state name:(const char *)name;

/**
 获取全局变量并入栈

 @param state 状态
 @param name 名称
 */
+ (void)getGlobal:(lua_State *)state name:(const char *)name;

/**
 解析字符串

 @param state 状态
 @param string lua脚本
 @return 执行结果
 */
+ (int)loadString:(lua_State *)state string:(const char *)string;

/**
 解析lua文件

 @param state 状态
 @param path lua文件路径
 @return 执行结果
 */
+ (int)loadFile:(lua_State *)state path:(const char *)path;

/**
 调用方法

 @param state 状态
 @param nargs 参数个数
 @param nresults 返回值个数
 @param errfunc 错误处理方法
 @return 执行结果
 */
+ (int)pCall:(lua_State *)state nargs:(int)nargs nresults:(int)nresults errfunc:(int)errfunc;

/**
 将栈索引调转换为基于栈底的表示方法
 
 @param index 栈索引
 @param state 状态
 @return 转换后索引
 */
+ (int)absIndex:(int)index state:(lua_State *)state;

/**
 获取栈顶位置
 
 @param state 状态
 @return 栈顶位置
 */
+ (int)getTop:(lua_State *)state;

/**
 出栈数据

 @param state 状态
 @param count 出栈元素数量
 */
+ (void)pop:(lua_State *)state count:(int)count;

/**
 判断是否为空类型

 @param state 状态
 @param index 栈索引
 @return YES 表示为空，否则不是
 */
+ (BOOL)isNil:(lua_State *)state index:(int)index;

/**
 判断是否为Table类型

 @param state 状态
 @param index 栈索引
 @return YES 表示为Table类型，否则不是
 */
+ (BOOL)isTable:(lua_State *)state index:(int)index;

/**
 判断是否为方法类型

 @param state 状态
 @param index 栈索引
 @return YES 表示为方法类型，否则不是
 */
+ (BOOL)isFunction:(lua_State *)state index:(int)index;

/**
 判断是否为Userdata类型

 @param state 状态
 @param index 栈索引
 @return YES 表示为Userdata类型，否则不是
 */
+ (BOOL)isUserdata:(lua_State *)state index:(int)index;

/**
 入栈一个Nil类型数据

 @param state 状态
 */
+ (void)pushNil:(void *)state;

/**
 入栈一个整型值

 @param integer 整型值
 @param state 状态
 */
+ (void)pushInteger:(lua_Integer)integer state:(lua_State *)state;

/**
 入栈一个数值

 @param number 数值
 @param state 状态
 */
+ (void)pushNumber:(lua_Number)number state:(lua_State *)state;

/**
 入栈一个布尔值

 @param boolean 布尔值
 @param state 状态
 */
+ (void)pushBoolean:(int)boolean state:(lua_State *)state;

/**
 入栈一个轻量级用户数据

 @param userdata 用户数据
 @param state 状态
 */
+ (void)pushLightUserdata:(void *)userdata state:(lua_State *)state;

/**
 入栈一个字符串

 @param string 字符串
 @param state 状态
 */
+ (void)pushString:(const char *)string state:(lua_State *)state;

/**
 入栈一个指定长度的字符串

 @param string 字符串
 @param len 长度
 @param state 状态
 */
+ (void)pushString:(const char *)string len:(size_t)len state:(lua_State *)state;

/**
 入栈一个C方法

 @param cfunction C方法
 @param state 状态
 */
+ (void)pushCFunction:(lua_CFunction)cfunction state:(lua_State *)state;

/**
 入栈一个C闭包

 @param cclosure C闭包
 @param n 闭包的环境变量数量
 @param state 状态
 */
+ (void)pushCClosure:(lua_CFunction)cclosure n:(int)n state:(lua_State *)state;

/**
 入栈栈中的指定值

 @param index 值所在索引
 @param state 状态
 */
+ (void)pushValue:(int)index state:(lua_State *)state;

/**
 转换为闭包环境变量索引

 @param index 索引
 @return 闭包变量索引
 */
+ (int)upvalueIndex:(int)index;


/**
 转换为整型类型

 @param state 状态
 @param index 索引
 @return 整型数值
 */
+ (lua_Integer)toInteger:(lua_State *)state index:(int)index;

/**
 转换为数值类型

 @param state 状态
 @param index 索引
 @return 数值
 */
+ (lua_Number)toNumber:(lua_State *)state index:(int)index;

/**
 转换为布尔类型

 @param state 状态
 @param index 索引
 @return 整型数值
 */
+ (int)toBoolean:(lua_State *)state index:(int)index;

/**
 转换为指针类型

 @param state 状态
 @param index 索引
 @return 指针对象
 */
+ (const void *)toPointer:(lua_State *)state index:(int)index;

/**
 转换为用户数据

 @param state 状态
 @param index 索引
 @return 用户数据对象
 */
+ (void *)toUserdata:(lua_State *)state index:(int)index;

/**
 转换为字符串

 @param state 状态
 @param index 索引
 @return 字符串
 */
+ (const char *)toString:(lua_State *)state index:(int)index;

/**
 转换为字符串

 @param state 状态
 @param index 索引
 @param len 字符串长度
 @return 字符串
 */
+ (const char *)toString:(lua_State *)state index:(int)index len:(size_t *)len;



/**
 创建Table类型数据

 @param state 状态
 */
+ (void)newTable:(lua_State *)state;

/**
 创建Userdata类型数据

 @param state 状态
 @param size 数据长度
 */
+ (void *)newUserdata:(lua_State *)state size:(size_t)size;

/**
 创建元表

 @param state 状态
 @param name 元表名称
 @return 执行状态
 */
+ (int)newMetatable:(lua_State *)state name:(const char *)name;

/**
 获取字段值并入栈

 @param state 状态
 @param index 对象索引
 @param name 字段名称
 */
+ (void)getField:(lua_State *)state index:(int)index name:(const char *)name;

/**
 设置字段值

 @param state 状态
 @param index 对象索引
 @param name 字段名称
 */
+ (void)setField:(lua_State *)state index:(int)index name:(const char *)name;

/**
 获取元表

 @param state 状态
 @param name 元表名称
 */
+ (void)getMetatable:(lua_State *)state name:(const char *)name;

/**
 获取元表

 @param state 状态
 @param index 元表索引
 
 @return 执行结果
 */
+ (int)getMetatable:(lua_State *)state index:(int)index;

/**
 设置元表

 @param state 状态
 @param index 元表位置索引
 @return 执行状态
 */
+ (int)setMetatable:(lua_State *)state index:(int)index;

/**
 设置表数据的源操作，不触发newIndex元方法

 @param state 状态
 @param index table对象位置索引
 @param n 设置值所在table的位置索引
 */
+ (void)rawSetI:(lua_State *)state index:(int)index n:(int)n;


/**
 设置表数据的源操作，不触发newIndex元方法

 @param state 状态
 @param index table对象位置索引
 */
+ (void)rawSet:(lua_State *)state index:(int)index;


/**
 获取表数据的源操作，不出发index元方法

 @param state 状态
 @param index table对象位置索引
 */
+ (void)rawGet:(lua_State *)state index:(int)index;

/**
 获取栈中指定位置类型

 @param state 状态
 @param index 栈索引
 @return 类型
 */
+ (int)type:(lua_State *)state index:(int)index;

/**
 获取下一对键值

 @param state 状态
 @param index table位置索引
 @return 执行状态
 */
+ (int)next:(lua_State *)state index:(int)index;

/**
 将栈顶元素插入指定位置

 @param state 状态
 @param index 索引
 */
+ (void)insert:(lua_State *)state index:(int)index;

/**
 删除栈中指定元素

 @param state 状态
 @param index 索引
 */
+ (void)remove:(lua_State *)state index:(int)index;

/**
 检测并转换栈中指定位置是否为字符串

 @param state 状态
 @param index 索引
 @return 字符串
 */
+ (const char *)checkString:(lua_State *)state index:(int)index;

/**
 抛出异常

 @param state 状态
 @param message 错误消息
 
 @return 执行状态
 */
+ (int)error:(lua_State *)state message:(const char *)message;


/**
 创建新线程

 @param state lua状态
 @return 线程状态
 */
+ (lua_State *)newThread:(lua_State *)state;


/**
 恢复线程

 @param state 线程状态
 @param fromThreadState 表示从哪条线程进行延续，没有可以设置为NULL
 @param argCount 参数数量
 @return 返回参数数量
 */
+ (int)resumeThread:(lua_State *)state
    fromThreadState:(lua_State *)fromThreadState
           argCount:(int)argCount;

/**
 挂起线程

 @param state 线程状态
 @param resultCount 返回结果数量
 @return 无
 */
+ (int)yielyThread:(lua_State *)state
       resultCount:(int)resultCount;


/**
 保护执行过程

 @param state 状态
 @param func 处理过程
 @param userdata 用户数据
 @return 执行状态，成功返回LUA_OK
 */
+ (int)rawRunProtected:(lua_State *)state
                  func:(Pfunc)func
              userdata:(void *)userdata;


/**
 设置钩子

 @param state 状态
 @param hook 钩子函数
 @param mask 钩子类型, 为0时表示取消钩子
 @param count 数量，仅用于类型LUA_MASKCOUNT
 */
+ (void)setHook:(lua_State *)state
           hook:(lua_Hook)hook
           mask:(int)mask
          count:(int)count;

@end
