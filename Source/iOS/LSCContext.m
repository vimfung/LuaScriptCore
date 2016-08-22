//
//  LUAContext.m
//  LuaSample
//
//  Created by vimfung on 16/7/13.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCContext.h"
#import "lua.h"
#import "lualib.h"
#import "lauxlib.h"
#import "luaconf.h"
#import "LSCValue_Private.h"

@interface LSCContext ()

/**
 *  Lua解析器
 */
@property (nonatomic) NameDef(lua_State) *state;

/**
 *  异常处理器
 */
@property (nonatomic, strong) LSCExceptionHandler exceptionHandler;

/**
 *  方法处理器集合
 */
@property (nonatomic, strong) NSMutableDictionary *methodBlocks;

@end

@implementation LSCContext

- (instancetype)init
{
    if (self = [super init])
    {
        self.methodBlocks = [NSMutableDictionary dictionary];
        
        self.state = NameDef(luaL_newstate)();
        //加载标准库
        NameDef(luaL_openlibs)(self.state);
        
        //设置搜索路径
        [self setSearchPath:[NSString stringWithFormat:@"%@/?.lua", [[NSBundle mainBundle] resourcePath]]];
    }
    
    return self;
}

- (void)dealloc
{
    NameDef(lua_close)(self.state);
}

- (void)onException:(LSCExceptionHandler)handler
{
    self.exceptionHandler = handler;
}

- (LSCValue *)evalScriptFromString:(NSString *)string
{
    int curTop = NameDef(lua_gettop)(self.state);
    int ret = NameDef(luaL_loadstring)(self.state, [string UTF8String]) || lua_pcall(self.state, 0, 1, 0);
    
    BOOL res = ret == 0;
    if (!res)
    {
        
        LSCValue *value = [self getValueByIndex:-1];
        NSString *errMessage = [value toString];
        
        if (self.exceptionHandler)
        {
            self.exceptionHandler (errMessage);
        }
        
        lua_pop(self.state, 1);
    }
    else
    {
        if (NameDef(lua_gettop)(self.state) > curTop)
        {
            //有返回值
            LSCValue *value = [self getValueByIndex:-1];
            lua_pop(self.state, 1);
            
            return value;
        }
    }
    
    return nil;
}

- (id)evalScriptFromFile:(NSString *)path
{
    int curTop = NameDef(lua_gettop)(self.state);
    int ret = luaL_loadfile(self.state, [path UTF8String]) || lua_pcall(self.state, 0, 0, 0);
    
    BOOL res = ret == 0;
    if (!res)
    {
        LSCValue *value = [self getValueByIndex:-1];
        NSString *errMessage = [value toString];
        if (self.exceptionHandler)
        {
            self.exceptionHandler (errMessage);
        }
        
        lua_pop(self.state, 1);
    }
    else
    {
        if (NameDef(lua_gettop)(self.state) > curTop)
        {
            //有返回值
            LSCValue *value = [self getValueByIndex:-1];
            lua_pop(self.state, 1);
            
            return value;
        }
    }
    
    return nil;
}

- (LSCValue *)callMethodWithName:(NSString *)methodName arguments:(NSArray *)arguments
{
    LSCValue *resultValue = nil;

    NameDef(lua_getglobal)(self.state, [methodName UTF8String]);
    if (lua_isfunction(self.state, -1))
    {
        //如果为function则进行调用
        __weak LSCContext *theContext = self;
        [arguments enumerateObjectsUsingBlock:^(LSCValue * _Nonnull value, NSUInteger idx, BOOL * _Nonnull stop) {
            
            [value pushWithState:theContext.state];
            
        }];
        
        if (lua_pcall(self.state, (int)arguments.count, 1, 0) == 0)
        {
            //调用成功
            resultValue = [self getValueByIndex:-1];
        }
        else
        {
            //调用失败
            LSCValue *value = [self getValueByIndex:-1];
            NSString *errMessage = [value toString];
            
            if (self.exceptionHandler)
            {
                self.exceptionHandler (errMessage);
            }

        }
        
        lua_pop(self.state, 1);
    }
    else
    {
        //将变量从栈中移除
        lua_pop(self.state, 1);
    }
    
    return resultValue;
}

- (void)registerMethodWithName:(NSString *)methodName block:(LSCFunctionHandler)block
{
    if (![self.methodBlocks objectForKey:methodName])
    {
        [self.methodBlocks setObject:block forKey:methodName];
        
        NameDef(lua_pushlightuserdata)(self.state, (__bridge void *)self);
        NameDef(lua_pushstring)(self.state, [methodName UTF8String]);
        NameDef(lua_pushcclosure)(self.state, cfuncRouteHandler, 2);
        NameDef(lua_setglobal)(self.state, [methodName UTF8String]);
    }
    else
    {
        @throw [NSException exceptionWithName:@"Unabled register method" reason:@"The method of the specified name already exists!" userInfo:nil];
    }

}

#pragma mark - Private

static int cfuncRouteHandler (NameDef(lua_State) *state)
{
    LSCContext *context = (__bridge LSCContext *)NameDef(lua_touserdata)(state, lua_upvalueindex(1));
    NSString *methodName = [NSString stringWithUTF8String:lua_tostring(state, lua_upvalueindex(2))];
    
    LSCFunctionHandler handler = context.methodBlocks[methodName];
    if (handler)
    {
        int top = NameDef(lua_gettop)(state);
        NSMutableArray *arguments = [NSMutableArray array];
        for (int i = 0; i < top; i++)
        {
            LSCValue *value = [context getValueByIndex:- i - 1];
            [arguments insertObject:value atIndex:0];
        }
        
        LSCValue *retValue = handler (arguments);
        [retValue pushWithState:state];
    }
    
    return 1;
}

/**
 *  获取栈中的值
 *
 *  @param index 栈索引，负数为从栈顶开始索引，正数为从栈底开始索引
 *
 *  @return 值对象
 */
- (LSCValue *)getValueByIndex:(NSInteger)index
{
    LSCValue *value = nil;
    
    switch(NameDef(lua_type)(self.state, (int)index))
    {
        case LUA_TNIL:
        {
            value = [LSCValue nilValue];
            break;
        }
        case LUA_TBOOLEAN:
        {
            value = [LSCValue booleanValue:NameDef(lua_toboolean)(self.state, (int)index)];
            break;
        }
        case LUA_TNUMBER:
        {
            value = [LSCValue numberValue:@(lua_tonumber(self.state, (int)index))];
            break;
        }
        case LUA_TSTRING:
        {
            NSString *strValue = [NSString stringWithCString:lua_tostring(self.state, (int)index) encoding:NSUTF8StringEncoding];
            if (strValue)
            {
                //为NSString
                value = [LSCValue stringValue:strValue];
            }
            else
            {
                //为NSData
                size_t len = 0;
                const char *bytes = NameDef(lua_tolstring)(self.state, (int)index, &len);
                NSData *data = [NSData dataWithBytes:bytes length:len];
                
                value = [LSCValue dataValue:data];
            }
            
            break;
        }
        case LUA_TTABLE:
        {
            NSMutableDictionary *dictValue = [NSMutableDictionary dictionary];
            NSMutableArray *arrayValue = [NSMutableArray array];
            
            NameDef(lua_pushnil(self.state));
            while (NameDef(lua_next)(self.state, -2))
            {
                LSCValue *value = [self getValueByIndex:-1];
                LSCValue *key = [self getValueByIndex:-2];
                
                if (arrayValue)
                {
                    if (key.valueType != LSCValueTypeNumber)
                    {
                        //非数组对象，释放数组
                        arrayValue = nil;
                    }
                    else if (key.valueType == LSCValueTypeNumber)
                    {
                        NSInteger index = [[key toNumber] integerValue];
                        if (index <= 0)
                        {
                            //非数组对象，释放数组
                            arrayValue = nil;
                        }
                        else if (index - 1 != arrayValue.count)
                        {
                            //非数组对象，释放数组
                            arrayValue = nil;
                        }
                        else
                        {
                            [arrayValue addObject:[value toObject]];
                        }
                    }
                }
                
                [dictValue setObject:[value toObject] forKey:[key toString]];
                
                lua_pop(self.state, 1);
            }
            
            if (arrayValue)
            {
                value = [LSCValue arrayValue:arrayValue];
            }
            else
            {
                value = [LSCValue dictionaryValue:dictValue];
            }
            
            break;
        }
        default:
        {
            //默认为nil
            value = [LSCValue nilValue];
            break;
        }
    }
    
    return value;
}

/**
 *  设置搜索路径，避免脚本中的require无法找到文件
 *
 *  @param path 搜索路径
 */
- (void)setSearchPath:(NSString *)path
{
    NameDef(lua_getglobal)(self.state, "package");
    NameDef(lua_getfield)(self.state, -1, "path");
    
    //取出当前路径，并附加新路径
    NSMutableString *curPath = [NSMutableString stringWithUTF8String:lua_tostring(self.state, -1)];
    [curPath appendFormat:@";%@", path];
    
    lua_pop(self.state, 1);
    NameDef(lua_pushstring)(self.state, curPath.UTF8String);
    NameDef(lua_setfield)(self.state, -2, "path");
    lua_pop(self.state, 1);
}

@end
