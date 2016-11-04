//
//  LUAContext.m
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCContext.h"
#import "LSCContext_Private.h"
#import "LSCValue_Private.h"
#import "LSCModule_Private.h"
#import <objc/runtime.h>

@interface LSCContext ()

@property (nonatomic, strong) NSNumber *value;

@end

@implementation LSCContext

- (instancetype)init
{
    if (self = [super init])
    {
        self.methodBlocks = [NSMutableDictionary dictionary];
        
        self.state = luaL_newstate();
        
        lua_gc(self.state, LUA_GCSTOP, 0);
        
        //加载标准库
        luaL_openlibs(self.state);
        
        lua_gc(self.state, LUA_GCRESTART, 0);
        
        //设置搜索路径
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        [self addSearchPath:resourcePath];
        
    }
    
    return self;
}

- (void)dealloc
{
    lua_close(self.state);
}

- (void)onException:(LSCExceptionHandler)handler
{
    self.exceptionHandler = handler;
}

- (void)addSearchPath:(NSString *)path
{
    [self setSearchPath:[NSString stringWithFormat:@"%@/?.lua", path]];
}

- (LSCValue *)evalScriptFromString:(NSString *)string
{
    int curTop = lua_gettop(self.state);
    int ret = luaL_loadstring(self.state, [string UTF8String]) ||
    lua_pcall(self.state, 0, 1, 0);
    
    LSCValue *retValue = nil;
    BOOL res = ret == 0;
    if (!res)
    {
        LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
        NSString *errMessage = [value toString];
        
        if (self.exceptionHandler)
        {
            self.exceptionHandler(errMessage);
        }
        
        lua_pop(self.state, 1);
    }
    else
    {
        if (lua_gettop(self.state) > curTop)
        {
            //有返回值
            retValue = [LSCValue valueWithContext:self atIndex:-1];
            lua_pop(self.state, 1);
        }
    }
    
    //回收内存
    lua_gc(self.state, LUA_GCCOLLECT, 0);
    
    return retValue;
}

- (id)evalScriptFromFile:(NSString *)path
{
    int curTop = lua_gettop(self.state);
    int ret = luaL_loadfile(self.state, [path UTF8String]) ||
    lua_pcall(self.state, 0, 1, 0);
    
    LSCValue *retValue = nil;
    BOOL res = ret == 0;
    if (!res)
    {
        LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
        NSString *errMessage = [value toString];
        if (self.exceptionHandler)
        {
            self.exceptionHandler(errMessage);
        }
        
        lua_pop(self.state, 1);
    }
    else
    {
        if (lua_gettop(self.state) > curTop)
        {
            //有返回值
            retValue = [LSCValue valueWithContext:self atIndex:-1];
            lua_pop(self.state, 1);

        }
    }
    
    //回收内存
    lua_gc(self.state, LUA_GCCOLLECT, 0);
    
    return retValue;
}

- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments
{
    LSCValue *resultValue = nil;
    
    lua_getglobal(self.state, [methodName UTF8String]);
    if (lua_isfunction(self.state, -1))
    {
        //如果为function则进行调用
        __weak LSCContext *theContext = self;
        [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
             
             [value pushWithContext:theContext];
             
         }];
        
        if (lua_pcall(self.state, (int)arguments.count, 1, 0) == 0)
        {
            //调用成功
            resultValue = [LSCValue valueWithContext:self atIndex:-1];
        }
        else
        {
            //调用失败
            LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
            NSString *errMessage = [value toString];
            
            if (self.exceptionHandler)
            {
                self.exceptionHandler(errMessage);
            }
        }
        
        lua_pop(self.state, 1);
    }
    else
    {
        //将变量从栈中移除
        lua_pop(self.state, 1);
    }
    
    //内存回收
    lua_gc(self.state, LUA_GCCOLLECT, 0);
    
    return resultValue;
}

- (void)registerMethodWithName:(NSString *)methodName
                         block:(LSCFunctionHandler)block
{
    if (![self.methodBlocks objectForKey:methodName])
    {
        [self.methodBlocks setObject:block forKey:methodName];
        
        lua_pushlightuserdata(self.state, (__bridge void *)self);
        lua_pushstring(self.state, [methodName UTF8String]);
        lua_pushcclosure(self.state, cfuncRouteHandler, 2);
        lua_setglobal(self.state, [methodName UTF8String]);
    }
    else
    {
        @throw [NSException
                exceptionWithName:@"Unabled register method"
                reason:@"The method of the specified name already exists!"
                userInfo:nil];
    }
}

- (void)registerModuleWithClass:(Class)moduleClass
{
    if ([moduleClass isSubclassOfClass:[LSCModule class]])
    {
        [moduleClass _regModule:moduleClass context:self];
    }
    else
    {
         [self raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCModule' class!", NSStringFromClass(moduleClass)]];
    }
}

- (void)unregisterModuleWithClass:(Class)moduleClass
{
    if ([moduleClass isSubclassOfClass:[LSCModule class]])
    {
        [moduleClass _unregModule:moduleClass context:self];
    }
    else
    {
        [self raiseExceptionWithMessage:[NSString stringWithFormat:@"The '%@' module is not subclass of the 'LSCModule' class!", NSStringFromClass(moduleClass)]];
    }
}

#pragma mark - Private

- (void)raiseExceptionWithMessage:(NSString *)message
{
    if (self.exceptionHandler)
    {
        self.exceptionHandler (message);
    }
}

static int cfuncRouteHandler(lua_State *state)
{
    LSCContext *context = (__bridge LSCContext *)lua_topointer(state, lua_upvalueindex(1));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(2))];
    
    LSCFunctionHandler handler = context.methodBlocks[methodName];
    if (handler)
    {
        int top = lua_gettop(state);
        NSMutableArray *arguments = [NSMutableArray array];
        for (int i = 0; i < top; i++)
        {
            LSCValue *value = [LSCValue valueWithContext:context atIndex:-i - 1];
            [arguments insertObject:value atIndex:0];
        }
        
        LSCValue *retValue = handler(arguments);
        [retValue pushWithContext:context];
    }
    
    //释放内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return 1;
}

/**
 *  设置搜索路径，避免脚本中的require无法找到文件
 *
 *  @param path 搜索路径
 */
- (void)setSearchPath:(NSString *)path
{
    lua_getglobal(self.state, "package");
    lua_getfield(self.state, -1, "path");
    
    //取出当前路径，并附加新路径
    NSMutableString *curPath =
    [NSMutableString stringWithUTF8String:lua_tostring(self.state, -1)];
    [curPath appendFormat:@";%@", path];
    
    lua_pop(self.state, 1);
    lua_pushstring(self.state, curPath.UTF8String);
    lua_setfield(self.state, -2, "path");
    lua_pop(self.state, 1);
}

@end
