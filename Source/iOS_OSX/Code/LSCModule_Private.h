//
//  LSCModule_Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCModule.h"
#import "LSCContext.h"

/**
 本地类型标识
 */
static NSString *const NativeTypeKey = @"_nativeType";

/**
 模块类型
 */
static NSString *const NativeModuleType = @"module";

@interface LSCModule ()

/**
 获取模块名称

 @param cls 模块类型
 @return 名称
 */
+ (NSString *)_getModuleNameWithClass:(Class)cls;

/**
 *  获取Lua方法名称，需要过滤冒号后面所有内容以及带With、By、At等
 *
 *  @param name 原始方法
 *
 *  @return 方法
 */
+ (NSString *)_getLuaMethodNameWithName:(NSString *)name;

/**
 注册模块

 @param module  模块类型
 @param context 上下文对象
 */
+ (void)_regModule:(Class)module context:(LSCContext *)context;


/**
 导出模块的所有类方法，包括父类中定义的方法

 @param thiz 注册的模块类型
 @param module 导出方法的模块类型,可能是注册模块类或者其父类
 @param context         上下文对象
 @param filterMethodNames   过滤的方法名称
 */
+ (void)_exportModuleAllMethod:(Class)thiz
                        module:(Class)module
                       context:(LSCContext *)context
             filterMethodNames:(NSArray<NSString *> *)filterMethodNames;

/**
 导出模块的所有类方法

 @param thiz    注册的模块类型
 @param module  导出的模块类型
 @param context 上下文对象
 @param filterMethodNames 过滤的方法名称
 */
+ (void)_exportModuleMethod:(Class)thiz
                     module:(Class)module
                    context:(LSCContext *)context
          filterMethodNames:(NSArray<NSString *> *)filterMethodNames;

/**
 反注册模块

 @param module    模块类型
 @param context 上下文对象
 */
+ (void)_unregModule:(Class)module context:(LSCContext *)context;

@end
