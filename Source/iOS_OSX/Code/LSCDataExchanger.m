//
//  LSCDataExchanger.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/3.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import "LSCDataExchanger.h"
#import "LSCDataExchanger_Private.h"
#import "LSCSession_Private.h"
#import "LSCValue_Private.h"
#import "LSCTable+Private.h"
#import "LSCPointer.h"
#import "LSCFunction_Private.h"
#import "LSCTuple_Private.h"
#import "LSCCoroutine+Private.h"
#import "LSCManagedObjectProtocol.h"
#import "LSCEngineAdapter.h"
#import "LSCExportTypeDescriptor.h"

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
    lua_State *state = self.context.currentSession.state;
    return [self valueByStackIndex:index withState:state];
}

- (void)pushStackWithObject:(id)object
{
    [self pushStackWithObject:object
                        state:self.context.currentSession.state
                        queue:self.context.optQueue];
    
}

- (void)pushStackWithObject:(id)object
                  coroutine:(LSCCoroutine *)coroutine
{
    [self pushStackWithObject:object
                        state:coroutine.state
                        queue:nil];
}

- (void)getLuaObject:(id)nativeObject
{
    lua_State *state = self.context.currentSession.state;
    [self getLuaObject:nativeObject state:state queue:self.context.optQueue];
}

- (void)setLubObjectByStackIndex:(NSInteger)index
                        objectId:(NSString *)objectId
{
    lua_State *state = self.context.currentSession.state;
    [self setLubObjectByStackIndex:index
                          objectId:objectId
                             state:state];
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
                    return;
                case LSCValueTypePtr:
                    [self retainLuaObject:[(LSCValue *)nativeObject toPointer]];
                    return;
                case LSCValueTypeFunction:
                    [self retainLuaObject:[(LSCValue *)nativeObject toFunction]];
                    return;
                case LSCValueTypeMap:
                case LSCValueTypeArray:
                    objectId = [((LSCValue *)nativeObject) toTable].linkId;
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
                    return;
                case LSCValueTypePtr:
                    [self releaseLuaObject:[(LSCValue *)nativeObject toPointer]];
                    return;
                case LSCValueTypeFunction:
                    [self releaseLuaObject:[(LSCValue *)nativeObject toFunction]];
                    return;
                case LSCValueTypeMap:
                case LSCValueTypeArray:
                    objectId = [((LSCValue *)nativeObject) toTable].linkId;
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

- (void)pushStackWithValue:(LSCValue *)value
                     state:(lua_State *)state
                     queue:(LSCOperationQueue *)queue
{
    __weak typeof(self) theExchanger = self;
    void (^handler) (void) = ^{
        
        //先判断_vars_中是否存在对象，如果存在则直接返回表中对象
        switch (value.valueType)
        {
            case LSCValueTypeInteger:
                [LSCEngineAdapter pushInteger:[value toInteger] state:state];
                break;
            case LSCValueTypeNumber:
                [LSCEngineAdapter pushNumber:[value toDouble] state:state];
                break;
            case LSCValueTypeNil:
                [LSCEngineAdapter pushNil:state];
                break;
            case LSCValueTypeString:
                [LSCEngineAdapter pushString:[value toString].UTF8String state:state];
                break;
            case LSCValueTypeBoolean:
                [LSCEngineAdapter pushBoolean:[value toBoolean] state:state];
                break;
            case LSCValueTypeArray:
            case LSCValueTypeMap:
            {
                [theExchanger pushStackWithObject:[value toTable] state:state queue:queue];
                break;
            }
            case LSCValueTypeData:
            {
                NSData *data = [value toData];
                [LSCEngineAdapter pushString:data.bytes len:data.length state:state];
                break;
            }
            case LSCValueTypeObject:
            {
                [theExchanger pushStackWithObject:[value toObject] state:state queue:queue];
                break;
            }
            case LSCValueTypePtr:
            {
                [theExchanger pushStackWithObject:[value toPointer] state:state queue:queue];
                break;
            }
            case LSCValueTypeFunction:
            {
                [theExchanger pushStackWithObject:[value toFunction] state:state queue:queue];
                break;
            }
            case LSCValueTypeTuple:
            {
                [theExchanger pushStackWithTuple:[value toTuple] state:state];
                break;
            }
            default:
                [LSCEngineAdapter pushNil:state];
                break;
        }
    };
    
    if (queue)
    {
        [queue performAction:handler];
    }
    else
    {
        handler();
    }
}

/**
 入栈对象

 @param object 需要入栈的对象
 @param state lua状态
 */
- (void)pushStackWithObject:(id)object
                      state:(lua_State *)state
                      queue:(LSCOperationQueue *)queue
{
    __weak typeof(self) theExchanger = self;
    void (^handler) (void) = ^{
        
        if (object)
        {
            if ([object isKindOfClass:[LSCValue class]])
            {
                [theExchanger pushStackWithValue:object state:state queue:queue];
            }
            else if ([object isKindOfClass:[NSDictionary class]])
            {
                [theExchanger pushStackWithDictionary:object state:state queue:queue];
            }
            else if ([object isKindOfClass:[NSArray class]])
            {
                [theExchanger pushStackWithArray:object state:state queue:queue];
            }
            else if ([object isKindOfClass:[NSNumber class]])
            {
                [LSCEngineAdapter pushNumber:[object doubleValue] state:state];
            }
            else if ([object isKindOfClass:[NSString class]])
            {
                [LSCEngineAdapter pushString:[object UTF8String] state:state];
            }
            else if ([object isKindOfClass:[NSData class]])
            {
                [LSCEngineAdapter pushString:[object bytes] len:[object length] state:state];
            }
            else if ([object isKindOfClass:[LSCTuple class]])
            {
                [theExchanger pushStackWithTuple:object state:state];
            }
            else
            {
                //LSCFunction\LSCPointer\LSCTable\NSObject
                if ([object isKindOfClass:[LSCTable class]])
                {
                    LSCTable *table = object;
                    if (!table.linkId)
                    {
                        //无关联ID时需要将table中的字典/数组入栈
                        if (table.isArray)
                        {
                            [theExchanger pushStackWithArray:table.valueObject state:state queue:queue];
                        }
                        else
                        {
                            [theExchanger pushStackWithDictionary:table.valueObject state:state queue:queue];
                        }
                        return;
                    }
                }
                
                [theExchanger doActionInVarsTableWithState:state queue:queue block:^{
                    
                    NSString *objectId = nil;
                    if ([object conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
                    {
                        objectId = [(id<LSCManagedObjectProtocol>)object linkId];
                    }
                    else if ([object isKindOfClass:[LSCTable class]])
                    {
                        LSCTable *table = object;
                        objectId = table.linkId;
                    }
                    else
                    {
                        objectId = [NSString stringWithFormat:@"%p", object];
                    }
                    
                    [LSCEngineAdapter getField:state index:-1 name:objectId.UTF8String];
                    if ([LSCEngineAdapter isNil:state index:-1])
                    {
                        //弹出变量
                        [LSCEngineAdapter pop:state count:1];
                        
                        //_vars_表中没有对应对象引用，则创建对应引用对象
                        BOOL hasPushStack = NO;
                        if ([object conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
                        {
                            hasPushStack = [(id<LSCManagedObjectProtocol>)object pushWithState:state queue:queue];
                        }
                        else if ([theExchanger.context.exportsTypeManager checkExportsTypeWithObject:object])
                        {
                            //为导出类型，让对象与Lua层对象进行关联
                            [theExchanger.context.exportsTypeManager createLuaObjectByObject:object state:state queue:queue];
                            hasPushStack = YES;
                        }
                        
                        if (!hasPushStack)
                        {
                            //先为实例对象在lua中创建内存
                            LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter newUserdata:state
                                                                                          size:sizeof(LSCUserdataRef)];
                            //创建本地实例对象，赋予lua的内存块
                            ref -> value = (void *)CFBridgingRetain(object);
                            
                            //设置userdata的元表
                            [LSCEngineAdapter getMetatable:state name:"_ObjectReference_"];
                            if ([LSCEngineAdapter isNil:state index:-1])
                            {
                                [LSCEngineAdapter pop:state count:1];
                                
                                //尚未注册_ObjectReference,开始注册对象
                                [LSCEngineAdapter newMetatable:state name:"_ObjectReference_"];
                                
                                [LSCEngineAdapter pushCFunction:objectReferenceGCHandler state:state];
                                [LSCEngineAdapter setField:state index:-2 name:"__gc"];
                            }
                            [LSCEngineAdapter setMetatable:state index:-2];
                        }
                        
                        //放入_vars_表中
                        [LSCEngineAdapter pushValue:-1 state:state];
                        [LSCEngineAdapter setField:state index:-3 name:objectId.UTF8String];
                    }
                    
                    //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
                    [LSCEngineAdapter insert:state index:-3];
                }];
            }
        }
        else
        {
            [LSCEngineAdapter pushNil:state];
        }
        
    };
    
    if (queue)
    {
        [queue performAction:handler];
    }
    else
    {
        handler ();
    }
}

/**
 入栈一个字典

 @param dictionary 字典
 */
- (void)pushStackWithDictionary:(NSDictionary *)dictionary
                          state:(lua_State *)state
                          queue:(LSCOperationQueue *)queue
{
    __weak typeof(self) theExchanger = self;
    void (^handler)(void) = ^{
        
        [LSCEngineAdapter newTable:state];
        
        [dictionary enumerateKeysAndObjectsUsingBlock:^(id _Nonnull key, id _Nonnull obj, BOOL *_Nonnull stop) {
            
            [theExchanger pushStackWithObject:obj state:state queue:queue];
            [LSCEngineAdapter setField:state index:-2 name:[[NSString stringWithFormat:@"%@", key] UTF8String]];
            
        }];
        
    };
    
    if (queue)
    {
        [queue performAction:handler];
    }
    else
    {
        handler();
    }
}

/**
 入栈一个数组

 @param array 数组
 */
- (void)pushStackWithArray:(NSArray *)array state:(lua_State *)state queue:(LSCOperationQueue *)queue
{
    __weak typeof(self) theExchanger = self;
    [self.context.optQueue performAction:^{
        
        [LSCEngineAdapter newTable:state];
        
        [array enumerateObjectsUsingBlock:^(id _Nonnull obj, NSUInteger idx, BOOL *_Nonnull stop) {
            
            // lua数组下标从1开始
            [theExchanger pushStackWithObject:obj state:state queue:queue];
            [LSCEngineAdapter rawSetI:state index:-2 n:(int)idx + 1];
            
        }];
        
    }];
}

/**
 入栈元组

 @param tuple 元组
 @param state 状态
 */
- (void)pushStackWithTuple:(LSCTuple *)tuple state:(lua_State *)state
{
    __weak typeof(self) theDataExchanger = self;
    [tuple.returnValues enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        
        LSCValue *value = [LSCValue objectValue:obj];
        [theDataExchanger pushStackWithObject:value];
        
    }];
}


/**
 在_vars_表中操作
 
 @param state 状态
 @param block 操作行为
 */
- (void)doActionInVarsTableWithState:(lua_State *)state
                               queue:(LSCOperationQueue *)queue
                               block:(void (^)(void))block
{
    void (^handler) (void) = ^{
        
        [LSCEngineAdapter getGlobal:state name:"_G"];
        if (![LSCEngineAdapter isTable:state index:-1])
        {
            [LSCEngineAdapter pop:state count:1];
            
            [LSCEngineAdapter newTable:state];
            
            [LSCEngineAdapter pushValue:-1 state:state];
            [LSCEngineAdapter setGlobal:state name:"_G"];
        }
        
        [LSCEngineAdapter getField:state index:-1 name:VarsTableName.UTF8String];
        if ([LSCEngineAdapter isNil:state index:-1])
        {
            [LSCEngineAdapter pop:state count:1];
            
            //创建引用表
            [LSCEngineAdapter newTable:state];
            
            //创建弱引用表元表
            [LSCEngineAdapter newTable:state];
            [LSCEngineAdapter pushString:"kv" state:state];
            [LSCEngineAdapter setField:state index:-2 name:"__mode"];
            [LSCEngineAdapter setMetatable:state index:-2];
            
            //放入全局变量_G中
            [LSCEngineAdapter pushValue:-1 state:state];
            [LSCEngineAdapter setField:state index:-3 name:VarsTableName.UTF8String];
        }
        
        if (block)
        {
            block ();
        }
        
        //弹出_vars_
        [LSCEngineAdapter pop:state count:1];
        
        //弹出_G
        [LSCEngineAdapter pop:state count:1];
        
    };
    
    if (queue)
    {
        [queue performAction:handler];
    }
    else
    {
        handler ();
    }
}

/**
 执行对象操作

 @param action 行为
 @param objectId 对象ID
 */
- (void)doAction:(LSCLuaObjectAction)action withObjectId:(NSString *)objectId
{
    [self.context.optQueue performAction:^{
        
        if (objectId)
        {
            lua_State *state = self.context.currentSession.state;
            
            [LSCEngineAdapter getGlobal:state name:"_G"];
            if ([LSCEngineAdapter isTable:state index:-1])
            {
                [LSCEngineAdapter getField:state index:-1 name:VarsTableName.UTF8String];
                if ([LSCEngineAdapter isTable:state index:-1])
                {
                    //检查对象是否在_vars_表中登记
                    [LSCEngineAdapter getField:state index:-1 name:objectId.UTF8String];
                    if (![LSCEngineAdapter isNil:state index:-1])
                    {
                        //检查_retainVars_表是否已经记录对象
                        [LSCEngineAdapter getField:state index:-3 name:RetainVarsTableName.UTF8String];
                        if (![LSCEngineAdapter isTable:state index:-1])
                        {
                            [LSCEngineAdapter pop:state count:1];
                            
                            //创建引用表
                            [LSCEngineAdapter newTable:state];
                            
                            //放入全局变量_G中
                            [LSCEngineAdapter pushValue:-1 state:state];
                            [LSCEngineAdapter setField:state index:-5 name:RetainVarsTableName.UTF8String];
                        }
                        
                        switch (action)
                        {
                            case LSCLuaObjectActionRetain:
                            {
                                //保留对象
                                //获取对象
                                [LSCEngineAdapter getField:state index:-1 name:objectId.UTF8String];
                                if ([LSCEngineAdapter isNil:state index:-1])
                                {
                                    [LSCEngineAdapter pop:state count:1];
                                    
                                    [LSCEngineAdapter newTable:state];
                                    
                                    //初始化引用次数
                                    [LSCEngineAdapter pushNumber:0 state:state];
                                    [LSCEngineAdapter setField:state index:-2 name:"retainCount"];
                                    
                                    [LSCEngineAdapter pushValue:-3 state:state];
                                    [LSCEngineAdapter setField:state index:-2 name:"object"];
                                    
                                    //将对象放入表中
                                    [LSCEngineAdapter pushValue:-1 state:state];
                                    
                                    [LSCEngineAdapter setField:state index:-3 name:objectId.UTF8String];
                                }
                                
                                //引用次数+1
                                [LSCEngineAdapter getField:state index:-1 name:"retainCount"];
                                lua_Integer retainCount = [LSCEngineAdapter toInteger:state index:-1];
                                [LSCEngineAdapter pop:state count:1];
                                
                                [LSCEngineAdapter pushNumber:retainCount + 1 state:state];
                                [LSCEngineAdapter setField:state index:-2 name:"retainCount"];
                                
                                //弹出引用对象
                                [LSCEngineAdapter pop:state count:1];
                                break;
                            }
                            case LSCLuaObjectActionRelease:
                            {
                                //释放对象
                                //获取对象
                                [LSCEngineAdapter getField:state index:-1 name:objectId.UTF8String];
                                if (![LSCEngineAdapter isNil:state index:-1])
                                {
                                    //引用次数-1
                                    [LSCEngineAdapter getField:state index:-1 name:"retainCount"];
                                    lua_Integer retainCount = [LSCEngineAdapter toInteger:state index:-1];
                                    [LSCEngineAdapter pop:state count:1];
                                    
                                    if (retainCount - 1 > 0)
                                    {
                                        [LSCEngineAdapter pushNumber:retainCount - 1 state:state];
                                        [LSCEngineAdapter setField:state index:-2 name:"retainCount"];
                                    }
                                    else
                                    {
                                        //retainCount<=0时移除对象引用
                                        [LSCEngineAdapter pushNil:state];
                                        [LSCEngineAdapter setField:state index:-3 name:objectId.UTF8String];
                                    }
                                }
                                
                                //弹出引用对象
                                [LSCEngineAdapter pop:state count:1];
                                break;
                            }
                            default:
                                break;
                        }
                        
                        //弹出_retainVars_
                        [LSCEngineAdapter pop:state count:1];
                    }
                    //弹出变量
                    [LSCEngineAdapter pop:state count:1];
                }
                
                //弹出_vars_
                [LSCEngineAdapter pop:state count:1];
            }
            
            //弹出_G
            [LSCEngineAdapter pop:state count:1];
        }
        
    }];
    
}

- (LSCValue *)valueByStackIndex:(int)index withState:(lua_State *)state
{
    
    __block LSCValue *value = nil;
    
    [self.context.optQueue performAction:^{
        
        int stackIndex = [LSCEngineAdapter absIndex:index state:state];
        
        NSString *objectId = nil;
        
        
        int type = [LSCEngineAdapter type:state index:stackIndex];
        
        switch (type)
        {
            case LUA_TNIL:
            {
                value = [LSCValue nilValue];
                break;
            }
            case LUA_TBOOLEAN:
            {
                value = [LSCValue booleanValue:[LSCEngineAdapter toBoolean:state index:stackIndex]];
                break;
            }
            case LUA_TNUMBER:
            {
                value = [LSCValue numberValue:@([LSCEngineAdapter toNumber:state index:stackIndex])];
                break;
            }
            case LUA_TSTRING:
            {
                size_t len = 0;
                const char *bytes = [LSCEngineAdapter toString:state index:stackIndex len:&len];
                
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
                //判断是否为类型
                [LSCEngineAdapter getField:state index:stackIndex name:"_nativeType"];
                if ([LSCEngineAdapter type:state index:-1] == LUA_TLIGHTUSERDATA)
                {
                    //为导出类型
                    LSCExportTypeDescriptor *typeDescriptor = (LSCExportTypeDescriptor *)[LSCEngineAdapter toPointer:state index:-1];
                    value = [LSCValue typeValue:typeDescriptor];
                    
                    [LSCEngineAdapter pop:state count:1];
                }
                else
                {
                    //出棧之前结果
                    [LSCEngineAdapter pop:state count:1];
                    
                    LSCTable *table = nil;
                    const void *userdata = [LSCEngineAdapter toPointer:state index:-1];
                    objectId = [NSString stringWithFormat:@"%p", userdata];
                    
                    //为Table数据
                    NSMutableDictionary *dictValue = [NSMutableDictionary dictionary];
                    NSMutableArray *arrayValue = [NSMutableArray array];
                    
                    [LSCEngineAdapter pushNil:state];
                    while ([LSCEngineAdapter next:state index:stackIndex])
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
                        
                        [LSCEngineAdapter pop:state count:1];
                    }
                    
                    if (arrayValue && arrayValue.count > 0)
                    {
                        table = [[LSCTable alloc] initWithArray:arrayValue
                                                       objectId:objectId
                                                        context:self.context];
                    }
                    else
                    {
                        table = [[LSCTable alloc] initWithDictionary:dictValue
                                                            objectId:objectId
                                                             context:self.context];
                    }
                    
                    value = [LSCValue tableValue:table];
                }
                
                break;
            }
            case LUA_TLIGHTUSERDATA:
            {
                LSCUserdataRef userdataRef = (LSCUserdataRef)([LSCEngineAdapter toPointer:state index:stackIndex]);
                LSCPointer *pointer = [[LSCPointer alloc] initWithUserdata:userdataRef];
                value = [LSCValue pointerValue:pointer];
                
                objectId = [(id<LSCManagedObjectProtocol>)pointer linkId];
                break;
            }
            case LUA_TUSERDATA:
            {
                LSCUserdataRef userdataRef = (LSCUserdataRef)[LSCEngineAdapter toUserdata:state index:stackIndex];
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
                LSCFunction *func = [[LSCFunction alloc] initWithContext:self.context index:stackIndex];
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
            [self setLubObjectByStackIndex:stackIndex objectId:objectId state:state];
        }
        
    }];
    
    
    return value;
}

/**
 设置Lua对象到_vars_表

 @param index 对应state中的栈索引
 @param objectId 对象标识
 @param state 状态
 */
- (void)setLubObjectByStackIndex:(NSInteger)index
                        objectId:(NSString *)objectId
                           state:(lua_State *)state
{
    LSCOperationQueue *queue = self.context.optQueue;
    [queue performAction:^{
        
        int stackIndex = [LSCEngineAdapter absIndex:(int)index state:state];
        [self doActionInVarsTableWithState:state queue:queue block:^{
            
            //放入对象到_vars_表中
            [LSCEngineAdapter pushValue:(int)stackIndex state:state];
            [LSCEngineAdapter setField:state index:-2 name:objectId.UTF8String];
            
        }];
        
    }];
    
}


/**
 获取Lua对象并入栈

 @param nativeObject 原生对象
 @param state 状态
 */
- (void)getLuaObject:(id)nativeObject
               state:(lua_State *)state
               queue:(LSCOperationQueue *)queue
{
    if (nativeObject)
    {
        NSString *objectId = nil;
        
        if ([nativeObject isKindOfClass:[LSCValue class]])
        {
            switch (((LSCValue *)nativeObject).valueType)
            {
                case LSCValueTypeObject:
                    [self getLuaObject:[(LSCValue *)nativeObject toObject] state:state queue:queue];
                    break;
                case LSCValueTypePtr:
                    [self getLuaObject:[(LSCValue *)nativeObject toPointer] state:state queue:queue];
                    break;
                case LSCValueTypeFunction:
                    [self getLuaObject:[(LSCValue *)nativeObject toFunction] state:state queue:queue];
                    break;
                case LSCValueTypeMap:
                case LSCValueTypeArray:
                    objectId = [((LSCValue *)nativeObject) toTable].linkId;
                    break;
                default:
                    break;
            }
        }
        else if ([nativeObject conformsToProtocol:@protocol(LSCManagedObjectProtocol)])
        {
            objectId = [nativeObject linkId];
        }
        else if ([nativeObject isKindOfClass:[LSCTable class]])
        {
            objectId = ((LSCTable *)nativeObject).linkId;
        }
        else
        {
            objectId = [NSString stringWithFormat:@"%p", nativeObject];
        }
        
        if (objectId)
        {
            [self doActionInVarsTableWithState:state queue:queue block:^{
                
                void (^handler)(void) = ^{
                   
                    [LSCEngineAdapter getField:state index:-1 name:objectId.UTF8String];
                    
                    //将值放入_G之前，目的为了让doActionInVarsTable将_vars_和_G出栈，而不影响该变量值入栈回传Lua
                    [LSCEngineAdapter insert:state index:-3];
                    
                };
                
                if (queue)
                {
                    [queue performAction:handler];
                }
                else
                {
                    handler();
                }
                
            }];
        }
    }
}

/**
 对象引用回收处理
 
 @param state Lua状态机
 
 @return 返回值数量
 */
static int objectReferenceGCHandler(lua_State *state)
{
    LSCUserdataRef ref = (LSCUserdataRef)[LSCEngineAdapter toUserdata:state index:1];
    //释放对象
    CFBridgingRelease(ref -> value);
    
    return 0;
}

@end
