//
//  LSCDataExchanger.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCDataExchanger.h"
#import "LSCContext_Private.h"
#import "LSCValue.h"
#import "LSCPointer.h"
#import "LSCFunction_Private.h"
#import "LSCTuple_Private.h"
#import "LSCManagedObjectProtocol.h"


/**
 Lua对象行为

 - LSCLuaObjectActionUnknown: 未知
 - LSCLuaObjectActionRetain: 保留
 - LSCLuaObjectActionRelease: 释放
 */
typedef NS_ENUM(NSUInteger, LSCLuaObjectAction)
{
    LSCLuaObjectActionUnknown = 0,
    LSCLuaObjectActionRetain = 1,
    LSCLuaObjectActionRelease = 2,
};

/**
 记录导入原生层的Lua引用变量表名称
 */
static NSString *const VarsTableName = @"_vars_";

/**
 记录保留的Lua对象变量表名称
 */
static NSString *const RetainVarsTableName = @"_retainVars_";

@interface LSCDataExchanger ()

/**
 上下文对象
 */
@property (nonatomic, weak) LSCContext *context;

@end

@implementation LSCDataExchanger

- (instancetype)initWithContext:(LSCContext *)context
{
    if (self = [super init])
    {
        self.context = context;
    }
    return self;
}

- (LSCValue *)valueByStackIndex:(int)index
{
    lua_State *state = self.context.state;
    index = lua_absindex(state, (int)index);
    
    NSString *objectId = nil;
    LSCValue *value = nil;
    
    int type = lua_type(state, index);
    
    switch (type)
    {
        case LUA_TNIL:
        {
            value = [LSCValue nilValue];
            break;
        }
        case LUA_TBOOLEAN:
        {
            value = [LSCValue booleanValue:lua_toboolean(state, (int)index)];
            break;
        }
        case LUA_TNUMBER:
        {
            value = [LSCValue numberValue:@(lua_tonumber(state, (int)index))];
            break;
        }
        case LUA_TSTRING:
        {
            size_t len = 0;
            const char *bytes = lua_tolstring(state, (int)index, &len);
            
            //尝试转换成字符串
            NSString *strValue = [NSString stringWithCString:bytes encoding:NSUTF8StringEncoding];
            if (strValue)
            {
                //为NSString
                value = [LSCValue stringValue:strValue];
            }
            else
            {
                //为NSData
                NSData *data = [NSData dataWithBytes:bytes length:len];
                value = [LSCValue dataValue:data];
            }
            
            break;
        }
        case LUA_TTABLE:
        {
            NSMutableDictionary *dictValue = [NSMutableDictionary dictionary];
            NSMutableArray *arrayValue = [NSMutableArray array];
            
            lua_pushnil(state);
            while (lua_next(state, (int)index))
            {
                LSCValue *value = [self valueByStackIndex:-1];
                LSCValue *key = [self valueByStackIndex:-2];
                
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
                
                lua_pop(state, 1);
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
        case LUA_TLIGHTUSERDATA:
        {
            LSCUserdataRef userdataRef = (LSCUserdataRef)(lua_topointer(state, (int)index));
            LSCPointer *pointer = [[LSCPointer alloc] initWithUserdata:userdataRef];
            value = [LSCValue pointerValue:pointer];
            
            objectId = [(id<LSCManagedObjectProtocol>)pointer linkId];
            break;
        }
        case LUA_TUSERDATA:
        {
            LSCUserdataRef userdataRef = (LSCUserdataRef)lua_touserdata(state, (int)index);
            id obj = (__bridge id)(userdataRef -> value);
            value = [LSCValue objectValue:obj];
            
            if ([obj conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
            {
                objectId = [obj linkId];
            }
            else
            {
                objectId = [NSString stringWithFormat:@"%p", obj];
            }
            break;
        }
        case LUA_TFUNCTION:
        {
            LSCFunction *func = [[LSCFunction alloc] initWithContext:self.context index:index];
            value = [LSCValue functionValue:func];
            
            objectId = [(id<LSCManagedObjectProtocol>)func linkId];
            break;
        }
        default:
        {
            //默认为nil
            value = [LSCValue nilValue];
            break;
        }
    }
    
    if (objectId && (type == LUA_TTABLE || type == LUA_TUSERDATA || type == LUA_TLIGHTUSERDATA || type == LUA_TFUNCTION))
    {
        //将引用对象放入表中
        [self setLubObjectByStackIndex:index objectId:objectId];
    }
    
    return value;
}

- (void)pushStackWithValue:(LSCValue *)value
{
    lua_State *state = self.context.state;
    
    //先判断_vars_中是否存在对象，如果存在则直接返回表中对象
    switch (value.valueType)
    {
        case LSCValueTypeInteger:
            lua_pushinteger(state, [value toInteger]);
            break;
        case LSCValueTypeNumber:
            lua_pushnumber(state, [value toDouble]);
            break;
        case LSCValueTypeNil:
            lua_pushnil(state);
            break;
        case LSCValueTypeString:
            lua_pushstring(state, [value toString].UTF8String);
            break;
        case LSCValueTypeBoolean:
            lua_pushboolean(state, [value toBoolean]);
            break;
        case LSCValueTypeArray:
        {
            [self pushStackWithObject:[value toArray]];
            break;
        }
        case LSCValueTypeMap:
        {
            [self pushStackWithObject:[value toDictionary]];
            break;
        }
        case LSCValueTypeData:
        {
            NSData *data = [value toData];
            lua_pushlstring(state, data.bytes, data.length);
            break;
        }
        case LSCValueTypeObject:
        {
            [self pushStackWithObject:[value toObject]];
            break;
        }
        case LSCValueTypePtr:
        {
            [self pushStackWithObject:[value toPointer]];
            break;
        }
        case LSCValueTypeFunction:
        {
            [self pushStackWithObject:[value toFunction]];
            break;
        }
        case LSCValueTypeTuple:
        {
            [[value toTuple] pushWithContext:self.context];
            break;
        }
        default:
            lua_pushnil(state);
            break;
    }
}

- (void)getLuaObject:(id)nativeObject
{
    if (nativeObject)
    {
        NSString *objectId = nil;
        
        if ([nativeObject isKindOfClass:[LSCValue class]])
        {
            switch (((LSCValue *)nativeObject).valueType)
            {
                case LSCValueTypeObject:
                    [self getLuaObject:[(LSCValue *)nativeObject toObject]];
                    break;
                case LSCValueTypePtr:
                    [self getLuaObject:[(LSCValue *)nativeObject toPointer]];
                    break;
                case LSCValueTypeFunction:
                    [self getLuaObject:[(LSCValue *)nativeObject toFunction]];
                    break;
                default:
                    break;
            }
        }
        else if ([nativeObject conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
        {
            objectId = [nativeObject linkId];
        }
        else
        {
            objectId = [NSString stringWithFormat:@"%p", nativeObject];
        }
        
        if (objectId)
        {
            lua_State *state = self.context.state;
            
            [self doActionInVarsTable:^{
                
                lua_getfield(state, -1, objectId.UTF8String);
                
                //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
                lua_insert(state, -3);
            }];
        }
    }
}

- (void)setLubObjectByStackIndex:(NSInteger)index objectId:(NSString *)objectId
{
    lua_State *state = self.context.state;
    
    [self doActionInVarsTable:^{
        
        //放入对象到_vars_表中
        lua_pushvalue(state, (int)index);
        lua_setfield(state, -2, objectId.UTF8String);
        
    }];
}

- (void)retainLuaObject:(id)nativeObject
{
    if (nativeObject)
    {
        NSString *objectId = nil;
        
        if ([nativeObject isKindOfClass:[LSCValue class]])
        {
            switch (((LSCValue *)nativeObject).valueType)
            {
                case LSCValueTypeObject:
                    [self retainLuaObject:[(LSCValue *)nativeObject toObject]];
                    break;
                case LSCValueTypePtr:
                    [self retainLuaObject:[(LSCValue *)nativeObject toPointer]];
                    break;
                case LSCValueTypeFunction:
                    [self retainLuaObject:[(LSCValue *)nativeObject toFunction]];
                    break;
                default:
                    break;
            }
        }
        else if ([nativeObject conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
        {
            objectId = [nativeObject linkId];
        }
        else
        {
            objectId = [NSString stringWithFormat:@"%p", nativeObject];
        }
        
        [self doAction:LSCLuaObjectActionRetain withObjectId:objectId];
    }
}

- (void)releaseLuaObject:(id)nativeObject
{
    if (nativeObject)
    {
        NSString *objectId = nil;
        
        if ([nativeObject isKindOfClass:[LSCValue class]])
        {
            switch (((LSCValue *)nativeObject).valueType)
            {
                case LSCValueTypeObject:
                    [self releaseLuaObject:[(LSCValue *)nativeObject toObject]];
                    break;
                case LSCValueTypePtr:
                    [self releaseLuaObject:[(LSCValue *)nativeObject toPointer]];
                    break;
                case LSCValueTypeFunction:
                    [self releaseLuaObject:[(LSCValue *)nativeObject toFunction]];
                    break;
                default:
                    break;
            }
        }
        else if ([nativeObject conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
        {
            objectId = [nativeObject linkId];
        }
        else
        {
            objectId = [NSString stringWithFormat:@"%p", nativeObject];
        }
        
        [self doAction:LSCLuaObjectActionRelease withObjectId:objectId];
    }
}

#pragma mark - Private

/**
 入栈对象

 @param object 需要入栈的对象
 */
- (void)pushStackWithObject:(id)object
{
    lua_State *state = self.context.state;
    
    if (object)
    {
        if ([object isKindOfClass:[NSDictionary class]])
        {
            [self pushStackWithDictionary:object];
        }
        else if ([object isKindOfClass:[NSArray class]])
        {
            [self pushStackWithArray:object];
        }
        else if ([object isKindOfClass:[NSNumber class]])
        {
            lua_pushnumber(state, [object doubleValue]);
        }
        else if ([object isKindOfClass:[NSString class]])
        {
            lua_pushstring(state, [object UTF8String]);
        }
        else if ([object isKindOfClass:[NSData class]])
        {
            lua_pushlstring(state, [object bytes], [object length]);
        }
        else
        {
            //LSCFunction\LSCPointer\NSObject
            [self doActionInVarsTable:^{
                
                NSString *objectId = nil;
                if ([object conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
                {
                    objectId = [(id<LSCManagedObjectProtocol>)object linkId];
                }
                else
                {
                    objectId = [NSString stringWithFormat:@"%p", object];
                }
                
                lua_getfield(state, -1, objectId.UTF8String);
                if (lua_isnil(state, -1))
                {
                    //弹出变量
                    lua_pop(state, 1);
                    
                    //_vars_表中没有对应对象引用，则创建对应引用对象
                    BOOL hasPushStack = NO;
                    if ([object conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
                    {
                        hasPushStack = [(id<LSCManagedObjectProtocol>)object pushWithContext:self.context];
                    }
                    
                    if (!hasPushStack)
                    {
                        //先为实例对象在lua中创建内存
                        LSCUserdataRef ref = (LSCUserdataRef)lua_newuserdata(state, sizeof(LSCUserdataRef));
                        //创建本地实例对象，赋予lua的内存块
                        ref -> value = (void *)CFBridgingRetain(object);
                        
                        //设置userdata的元表
                        luaL_getmetatable(state, "_ObjectReference_");
                        if (lua_isnil(state, -1))
                        {
                            lua_pop(state, 1);
                            
                            //尚未注册_ObjectReference,开始注册对象
                            luaL_newmetatable(state, "_ObjectReference_");
                            
                            lua_pushcfunction(state, objectReferenceGCHandler);
                            lua_setfield(state, -2, "__gc");
                        }
                        lua_setmetatable(state, -2);
                        
                        //放入_vars_表中
                        lua_pushvalue(state, -1);
                        lua_setfield(state, -3, objectId.UTF8String);
                    }
                }
                
                //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
                lua_insert(state, -3);
            }];
        }
    }
    else
    {
        lua_pushnil(state);
    }
}


/**
 入栈一个字典

 @param dictionary 字典
 */
- (void)pushStackWithDictionary:(NSDictionary *)dictionary
{
    lua_State *state = self.context.state;
    
    lua_newtable(state);
    
    __weak typeof(self) theExchanger = self;
    [dictionary enumerateKeysAndObjectsUsingBlock:^(id _Nonnull key, id _Nonnull obj, BOOL *_Nonnull stop) {
        
        [theExchanger pushStackWithObject:obj];
        lua_setfield(state, -2, [[NSString stringWithFormat:@"%@", key] UTF8String]);
        
    }];
}

/**
 入栈一个数组

 @param array 数组
 */
- (void)pushStackWithArray:(NSArray *)array
{
    lua_State *state = self.context.state;
    
    lua_newtable(state);
    
    __weak typeof(self) theExchanger = self;
    [array enumerateObjectsUsingBlock:^(id _Nonnull obj, NSUInteger idx, BOOL *_Nonnull stop) {
        
        // lua数组下标从1开始
        [theExchanger pushStackWithObject:obj];
        lua_rawseti(state, -2, idx + 1);
        
    }];
}


/**
 在_vars_表中操作

 @param block 操作行为
 */
- (void)doActionInVarsTable:(void (^)())block
{
    lua_State *state = self.context.state;
    
    lua_getglobal(state, "_G");
    if (!lua_istable(state, -1))
    {
        lua_pop(state, 1);
        
        lua_newtable(state);
        
        lua_pushvalue(state, -1);
        lua_setglobal(state, "_G");
    }
    
    lua_getfield(state, -1, VarsTableName.UTF8String);
    if (lua_isnil(state, -1))
    {
        lua_pop(state, 1);
        
        //创建引用表
        lua_newtable(state);
        
        //创建弱引用表元表
        lua_newtable(state);
        lua_pushstring(state, "kv");
        lua_setfield(state, -2, "__mode");
        lua_setmetatable(state, -2);
        
        //放入全局变量_G中
        lua_pushvalue(state, -1);
        lua_setfield(state, -3, VarsTableName.UTF8String);
    }
    
    if (block)
    {
        block ();
    }
    
    //弹出_vars_
    lua_pop(state, 1);
    
    //弹出_G
    lua_pop(state, 1);
}

/**
 执行对象操作

 @param action 行为
 @param objectId 对象ID
 */
- (void)doAction:(LSCLuaObjectAction)action withObjectId:(NSString *)objectId
{
    if (objectId)
    {
        lua_State *state = self.context.state;
        
        lua_getglobal(state, "_G");
        if (lua_istable(state, -1))
        {
            lua_getfield(state, -1, VarsTableName.UTF8String);
            if (lua_istable(state, -1))
            {
                //检查对象是否在_vars_表中登记
                lua_getfield(state, -1, objectId.UTF8String);
                if (!lua_isnil(state, -1))
                {
                    //检查_retainVars_表是否已经记录对象
                    lua_getfield(state, -3, RetainVarsTableName.UTF8String);
                    if (!lua_istable(state, -1))
                    {
                        lua_pop(state, 1);
                        
                        //创建引用表
                        lua_newtable(state);
                        
                        //放入全局变量_G中
                        lua_pushvalue(state, -1);
                        lua_setfield(state, -5, RetainVarsTableName.UTF8String);
                    }
                    
                    switch (action)
                    {
                        case LSCLuaObjectActionRetain:
                        {
                            //保留对象
                            //获取对象
                            lua_getfield(state, -1, objectId.UTF8String);
                            if (lua_isnil(state, -1))
                            {
                                lua_pop(state, 1);
                                
                                lua_newtable(state);
                                
                                //初始化引用次数
                                lua_pushnumber(state, 0);
                                lua_setfield(state, -2, "retainCount");
                                
                                lua_pushvalue(state, -3);
                                lua_setfield(state, -2, "object");
                                
                                //将对象放入表中
                                lua_pushvalue(state, -1);
                                lua_setfield(state, -3, objectId.UTF8String);
                            }
                            
                            //引用次数+1
                            lua_getfield(state, -1, "retainCount");
                            lua_Integer retainCount = lua_tointeger(state, -1);
                            lua_pop(state, 1);
                            
                            lua_pushnumber(state, retainCount+1);
                            lua_setfield(state, -2, "retainCount");
                            
                            //弹出引用对象
                            lua_pop(state, 1);
                            break;
                        }
                        case LSCLuaObjectActionRelease:
                        {
                            //释放对象
                            //获取对象
                            lua_getfield(state, -1, objectId.UTF8String);
                            if (!lua_isnil(state, -1))
                            {
                                //引用次数-1
                                lua_getfield(state, -1, "retainCount");
                                lua_Integer retainCount = lua_tointeger(state, -1);
                                lua_pop(state, 1);
                                
                                if (retainCount - 1 > 0)
                                {
                                    lua_pushnumber(state, retainCount - 1);
                                    lua_setfield(state, -2, "retainCount");
                                }
                                else
                                {
                                    //retainCount<=0时移除对象引用
                                    lua_pushnil(state);
                                    lua_setfield(state, -3, objectId.UTF8String);
                                }
                            }
                            
                            //弹出引用对象
                            lua_pop(state, 1);
                            break;
                        }
                        default:
                            break;
                    }
                    
                    //弹出_retainVars_
                    lua_pop(state, 1);
                }
                //弹出变量
                lua_pop(state, 1);
            }
            
            //弹出_vars_
            lua_pop(state, 1);
        }
        
        //弹出_G
        lua_pop(state, 1);
    }
}

/**
 对象引用回收处理
 
 @param state Lua状态机
 
 @return 返回值数量
 */
static int objectReferenceGCHandler(lua_State *state)
{
    LSCUserdataRef ref = (LSCUserdataRef)lua_touserdata(state, 1);
    //释放对象
    CFBridgingRelease(ref -> value);
    
    return 0;
}

@end
