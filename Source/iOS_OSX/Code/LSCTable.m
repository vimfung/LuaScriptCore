//
//  LSCTable.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/1/16.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import "LSCTable.h"
#import "LSCTable+Private.h"
#import "LSCSession_Private.h"

@implementation LSCTable

- (instancetype)initWithDictionary:(NSDictionary *)dictionary
                          objectId:(NSString *)objectId
                           context:(LSCContext *)context
{
    if (self = [super init])
    {
        _linkId = [objectId copy];
        self.context = context;
        self.valueObject = dictionary;
    }
    return self;
}

- (instancetype)initWithArray:(NSArray *)array
                     objectId:(NSString *)objectId
                      context:(LSCContext *)context
{
    if (self = [super init])
    {
        _linkId = [objectId copy];
        _isArray = YES;
        self.context = context;
        self.valueObject = array;
    }
    return self;
}

- (void)setObject:(id)object forKeyPath:(nonnull NSString *)keyPath
{
    if (!self.isArray)
    {
        NSArray *keys = [keyPath componentsSeparatedByString:@"."];
        
        NSMutableDictionary *dict = [self.valueObject mutableCopy];
        [self setObject:object toDict:dict forKeys:keys keyIndex:0];
        self.valueObject = dict;
            
        if (self.context)
        {
            //表示有对应的lua对象
            __weak typeof(self) theTable = self;
            [self.context.optQueue performAction:^{
                
                lua_State *state = theTable.context.currentSession.state;
                [theTable.context.dataExchanger getLuaObject:theTable];
                
                if ([LSCEngineAdapter type:state index:-1] == LUA_TTABLE)
                {
                    //先寻找对应的table对象
                    BOOL hasExists = YES;
                    if (keys.count > 1)
                    {
                        for (NSInteger i = 0; i < keys.count - 1; i++)
                        {
                            NSString *key = keys[i];
                            [theTable.context.dataExchanger pushStackWithObject:key];
                            [LSCEngineAdapter rawGet:state index:-2];
                            
                            if ([LSCEngineAdapter type:state index:-1] == LUA_TTABLE)
                            {
                                //移除前一个table对象
                                [LSCEngineAdapter remove:state index:-2];
                            }
                            else
                            {
                                hasExists = NO;
                                [LSCEngineAdapter pop:state count:1];
                                break;
                            }
                        }
                    }
                    
                    //设置对象
                    if (hasExists)
                    {
                        [theTable.context.dataExchanger pushStackWithObject:keys.lastObject];
                        [theTable.context.dataExchanger pushStackWithObject:object];
                        [LSCEngineAdapter rawSet:state index:-3];
                    }
                    
                }
                [LSCEngineAdapter pop:state count:1];
                
            }];
            
        }
        
    }
}

- (NSString *)description
{
    return [self.valueObject description];
}

#pragma mark - Private


/**
 设置对象到指定字典对象中，递归方法，用于逐层变更嵌套字典的值

 @param object 对象
 @param dict 字典
 @param keys 键名数组
 @param keyIndex 键名索引
 */
- (void)setObject:(id)object
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
                [self setObject:object toDict:subDict forKeys:keys keyIndex:keyIndex + 1];
                //重新设置回父级字典中
                [dict setObject:subDict forKey:key];
            }
        }
    }
}

@end
