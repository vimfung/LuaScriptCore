//
//  LSCPointer.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/10/27.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>


/**
 Userdata引用
 */
typedef struct
{
    void *value;
    
} *LSCUserdataRef;

/**
 Lua指针
 */
@interface LSCPointer : NSObject


/**
 初始化Lua指针对象

 @param ref userdata引用
 @return 指针对象
 */
- (instancetype)initWithUserdata:(LSCUserdataRef)ref;


/**
 初始化Lua指针对象

 @param ptr 指针
 @return 指针对象
 */
- (instancetype)initWithPtr:(const void *)ptr;


/**
 获取Userdata引用

 @return 引用对象
 */
- (const LSCUserdataRef)value;

@end
