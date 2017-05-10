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
#import "LSCTuple.h"
#import <objc/runtime.h>

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
        
        //初始化数据交换器
        self.dataExchanger = [[LSCDataExchanger alloc] initWithContext:self];
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

- (void)setGlobalWithValue:(LSCValue *)value forName:(NSString *)name
{
    [value pushWithContext:self];
    lua_setglobal(self.state, name.UTF8String);
}

- (LSCValue *)getGlobalForName:(NSString *)name
{
    lua_getglobal(self.state, name.UTF8String);
    return [LSCValue valueWithContext:self atIndex:-1];
}

- (void)retainValue:(LSCValue *)value
{
    [self.dataExchanger retainLuaObject:value];
}

- (void)releaseValue:(LSCValue *)value
{
    [self.dataExchanger releaseLuaObject:value];
}

- (LSCValue *)evalScriptFromString:(NSString *)string
{
    LSCValue *returnValue = nil;
    int curTop = lua_gettop(self.state);
    int returnCount = 0;
    
    luaL_loadstring(self.state, [string UTF8String]);
    if (lua_pcall(self.state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(self.state) - curTop;
        if (returnCount > 1)
        {
            LSCTuple *tuple = [[LSCTuple alloc] init];
            for (int i = 1; i <= returnCount; i++)
            {
                LSCValue *value = [LSCValue valueWithContext:self atIndex:curTop + i];
                [tuple addReturnValue:[value toObject]];
            }
            
            returnValue = [LSCValue tupleValue:tuple];
        }
        else if (returnCount == 1)
        {
            returnValue = [LSCValue valueWithContext:self atIndex:-1];
        }
    }
    else
    {
        //调用失败
        returnCount = 1;
        LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
        NSString *errMessage = [value toString];
        [self raiseExceptionWithMessage:errMessage];
    }
    
    //弹出返回值
    lua_pop(self.state, returnCount);
    
    if (!returnValue)
    {
        returnValue = [LSCValue nilValue];
    }
    
    //回收内存
    lua_gc(self.state, LUA_GCCOLLECT, 0);
    
    return returnValue;
}

- (LSCValue *)evalScriptFromFile:(NSString *)path
{
    if (!path)
    {
        NSString *errMessage = @"Lua file path is empty!";
        [self raiseExceptionWithMessage:errMessage];
        
        return nil;
    }
    
    if (![path hasPrefix:@"/"])
    {
        //应用包内路径
        path = [NSString stringWithFormat:@"%@/%@", [[NSBundle mainBundle] resourcePath], path];
    }
    
    LSCValue *retValue = nil;
    int curTop = lua_gettop(self.state);
    int returnCount = 0;
    
    luaL_loadfile(self.state, [path UTF8String]);
    if (lua_pcall(self.state, 0, LUA_MULTRET, 0) == 0)
    {
        //调用成功
        returnCount = lua_gettop(self.state) - curTop;
        if (returnCount > 1)
        {
            LSCTuple *tuple = [[LSCTuple alloc] init];
            for (int i = 1; i <= returnCount; i++)
            {
                LSCValue *value = [LSCValue valueWithContext:self atIndex:curTop + i];
                [tuple addReturnValue:[value toObject]];
            }
            retValue = [LSCValue tupleValue:tuple];
        }
        else if (returnCount == 1)
        {
            retValue = [LSCValue valueWithContext:self atIndex:-1];
        }
    }
    else
    {
        //调用失败
        returnCount = 1;
        LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
        NSString *errMessage = [value toString];
        [self raiseExceptionWithMessage:errMessage];
    }
    
    //弹出返回值
    lua_pop(self.state, returnCount);
    
    if (!retValue)
    {
        retValue = [LSCValue nilValue];
    }
    
    //回收内存
    lua_gc(self.state, LUA_GCCOLLECT, 0);
    
    return retValue;
}

- (LSCValue *)callMethodWithName:(NSString *)methodName
                       arguments:(NSArray<LSCValue *> *)arguments
{
    LSCValue *resultValue = nil;
    
    int curTop = lua_gettop(self.state);
    
    lua_getglobal(self.state, [methodName UTF8String]);
    if (lua_isfunction(self.state, -1))
    {
        int returnCount = 0;
        
        //如果为function则进行调用
        __weak LSCContext *theContext = self;
        [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
             
             [value pushWithContext:theContext];
             
         }];
        
        if (lua_pcall(self.state, (int)arguments.count, LUA_MULTRET, 0) == 0)
        {
            //调用成功
            returnCount = lua_gettop(self.state) - curTop;
            if (returnCount > 1)
            {
                LSCTuple *tuple = [[LSCTuple alloc] init];
                for (int i = 1; i <= returnCount; i++)
                {
                    LSCValue *value = [LSCValue valueWithContext:self atIndex:curTop + i];
                    [tuple addReturnValue:[value toObject]];
                }
                resultValue = [LSCValue tupleValue:tuple];
            }
            else if (returnCount == 1)
            {
                resultValue = [LSCValue valueWithContext:self atIndex:-1];
            }
        }
        else
        {
            //调用失败
            returnCount = 1;
            LSCValue *value = [LSCValue valueWithContext:self atIndex:-1];
            NSString *errMessage = [value toString];
            [self raiseExceptionWithMessage:errMessage];
        }
        
        lua_pop(self.state, returnCount);
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
    int count = 0;
    
    LSCContext *context = (__bridge LSCContext *)lua_topointer(state, lua_upvalueindex(1));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(2))];
    
    LSCFunctionHandler handler = context.methodBlocks[methodName];
    if (handler)
    {
        int top = lua_gettop(state);
        NSMutableArray *arguments = [NSMutableArray array];
        for (int i = 1; i <= top; i++)
        {
            LSCValue *value = [LSCValue valueWithContext:context atIndex:i];
            [arguments addObject:value];
        }
        
        LSCValue *retValue = handler(arguments);
        if (retValue.valueType == LSCValueTypeTuple)
        {
            count = (int)[retValue toTuple].count;
        }
        else
        {
            count = 1;
        }
        
        [retValue pushWithContext:context];
    }
    
    //释放内存
    lua_gc(state, LUA_GCCOLLECT, 0);
    
    return count;
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
