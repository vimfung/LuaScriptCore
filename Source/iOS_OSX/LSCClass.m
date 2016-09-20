//
//  LSCClass.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCClass.h"
#import "LSCClass_Private.h"

@implementation LSCClass

- (instancetype)initWithName:(NSString *)name
                   baseClass:(LSCClass *)baseClass
{
    if (self = [super init])
    {
        self.name = name;
        self.baseClass = baseClass;
        self.methodBlocks = [NSMutableDictionary dictionary];
        
        __weak LSCClass *theClass = self;
        self.registerHandler = ^(lua_State *state){
          
            [theClass defaultClassRegisterHandler:state];
            
        };
        
        if (!self.baseClass)
        {
            //默认继承于object类型
            self.baseClass = [LSCClass objectClass];
        }
    }
    
    return self;
}

+ (LSCClass *)objectClass
{
    static LSCClass *objecClass = nil;
    static dispatch_once_t predicate;
    
    dispatch_once(&predicate, ^{
       
        objecClass = [[LSCClass alloc] init];
        objecClass.name = @"Object";
        objecClass.registerHandler = ^(lua_State *state){
            
        };
        
    });
    
    return objecClass;
}

- (void)onCreate:(void (^)(LSCClassInstance *instance, NSArray *arguments))handler
{
    self.createHandler = handler;
}

- (void)onDestory:(void (^)(LSCClassInstance *instance))handler
{
    self.destoryHandler = handler;
}

- (void)registerInstanceMethodWithName:(NSString *)methodName
                                 block:(LSCValue* (^) (LSCClassInstance *instance, NSArray *arguments))block
{
    if (![self.methodBlocks objectForKey:methodName])
    {
        [self.methodBlocks setObject:block forKey:methodName];
    }
    else
    {
        @throw [NSException
                exceptionWithName:@"Unabled register method"
                reason:@"The method of the specified name already exists!"
                userInfo:nil];
    }
}

#pragma mark - Private

/**
 *  默认类注册方法事件处理器
 *
 *  @param state lua状态机
 */
- (void)defaultClassRegisterHandler:(lua_State *)state
{
    
}

/**
 *  对象类注册方法事件处理器
 *
 *  @param state lua状态机
 */
- (void)objectClassRegisterHandler:(lua_State *)state
{
    
}

@end
