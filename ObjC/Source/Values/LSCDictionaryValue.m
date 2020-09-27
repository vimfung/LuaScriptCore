//
//  LSCDictionaryValue.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/8/9.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCDictionaryValue.h"
#import "LSCValue.h"
#import "LSCApiAdapter.h"

@interface LSCDictionaryValue ()

@property (nonatomic, strong) NSMutableDictionary *rawValue;

/**
 table标识
 */
@property (nonatomic, copy, readonly) NSString *tableId;

/**
 所属上下文对象
 */
@property (nonatomic, weak, readonly) LSCContext *ownerContext;

@end

@implementation LSCDictionaryValue

- (instancetype)initWithDictionary:(NSDictionary *)dictionary
{
    if ([LSCDictionaryValue _checkType:dictionary])
    {
        return [self _initWithDictionary:dictionary];
    }
    
    return nil;
}

- (void)setObject:(id<LSCValueType>)object forKeyPath:(NSString *)keyPath
{
    NSArray *keys = [keyPath componentsSeparatedByString:@"."];
    
    NSMutableDictionary *dict = [self.rawValue mutableCopy];
    [self _setObject:object.rawValue toDict:dict forKeys:keys keyIndex:0];
    self.rawValue = dict;
    
    if (self.tableId)
    {
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        [apiAdapter setTableValue:object
                       forKeyPath:keyPath
                           withId:self.tableId
                          context:self.ownerContext];
    }
}

#pragma mark - LSCValueType

+ (instancetype)createValue:(id)rawValue
{
    if ([LSCDictionaryValue _checkType:rawValue])
    {
        return [[LSCDictionaryValue alloc] _initWithDictionary:rawValue];
    }
    
    return nil;
}

+ (instancetype)createValueWithContext:(LSCContext *)context stackIndex:(int)stackIndex
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    if ([apiAdapter getTypeWithStackIndex:stackIndex context:context] == LSCBasicTypeTable)
    {
        id tableValue = [apiAdapter getTableWithStackIndex:stackIndex context:context];
        if ([LSCDictionaryValue _checkType:tableValue])
        {
            NSString *tableId = [apiAdapter getLuaObjectIdWithStackIndex:stackIndex context:context];
            return [[LSCDictionaryValue alloc] _initWithDictionary:tableValue
                                                           tableId:tableId
                                                           context:context];
        }
    }
    
    return nil;
}

- (void)pushWithContext:(LSCContext *)context
{
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter pushDictionary:self.rawValue context:context];
}

#pragma mark - Rewrite

+ (void)load
{
    [LSCValue registerValueType:[LSCDictionaryValue class]];
}

- (instancetype)init
{
    return [self _initWithDictionary:@{}];
}

- (void)dealloc
{
    if (self.tableId)
    {
        //释放对象
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        [apiAdapter setLuaObjectWithId:self.tableId
                                option:LSCSetLuaObjectOptionRelease
                               context:self.ownerContext];
    }
}

#pragma mark - Private


/**
 检测类型是否符合DictionaryValue

 @param value 值
 @return YES 符合，NO 不符合
 */
+ (BOOL)_checkType:(id)value
{
    return [value isKindOfClass:[NSDictionary class]];
}

/**
 初始化

 @param dictionary 字典
 @return 实例对象
 */
- (instancetype)_initWithDictionary:(NSDictionary *)dictionary
{
    if (self = [super init])
    {
        self.rawValue = [dictionary mutableCopy];
    }
    
    return self;
}

/**
 初始化

 @param dictionary 字典
 @param tableId 对应lua中的table变量标识
 @param context 上下文
 @return 实例对象
 */
- (instancetype)_initWithDictionary:(NSDictionary *)dictionary
                            tableId:(NSString *)tableId
                            context:(LSCContext *)context
{
    if (self = [super init])
    {
        self.rawValue = [dictionary mutableCopy];
        _tableId = tableId;
        _ownerContext = context;
        
        //持有Lua对象
        LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
        [apiAdapter setLuaObjectWithId:self.tableId
                                option:LSCSetLuaObjectOptionRetain
                               context:context];
    }
    
    return self;
}

/**
 设置对象到指定字典对象中，递归方法，用于逐层变更嵌套字典的值
 
 @param object 对象
 @param dict 字典
 @param keys 键名数组
 @param keyIndex 键名索引
 */
- (void)_setObject:(id)object
            toDict:(NSMutableDictionary *)dict
           forKeys:(NSArray *)keys
          keyIndex:(NSInteger)keyIndex
{
    if (keyIndex < keys.count)
    {
        NSString *key = keys[keyIndex];
        if (keys.count == keyIndex + 1)
        {
            //最后一个元素
            if (object)
            {
                [dict setObject:object forKey:key];
            }
            else
            {
                [dict removeObjectForKey:key];
            }
        }
        else
        {
            id value = dict[key];
            if ([value isKindOfClass:[NSDictionary class]])
            {
                NSMutableDictionary *subDict = [value mutableCopy];
                [self _setObject:object toDict:subDict forKeys:keys keyIndex:keyIndex + 1];
                
                //重新设置回父级字典中
                [dict setObject:subDict forKey:key];
            }
        }
    }
}

@end
