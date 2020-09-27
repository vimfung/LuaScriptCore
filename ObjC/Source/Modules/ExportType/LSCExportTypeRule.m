//
//  LSCExportTypeRule.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/3.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCExportTypeRule.h"
#import "LSCDefaultExportFilter.h"

static Class _ruleClass = nil;
static LSCExportTypeRule *_defaultRule = nil;

@interface LSCExportTypeRule ()

/**
 过滤器集合
 */
@property (nonatomic, strong) NSMutableArray<id<LSCExportFilter>> *filters;

@end

@implementation LSCExportTypeRule

+ (void)registerRuleClass:(Class)ruleClass
{
    @synchronized (self)
    {
        if ([ruleClass isKindOfClass:[LSCExportTypeRule class]]
            && _ruleClass != ruleClass)
        {
            _ruleClass = ruleClass;
            _defaultRule = [[_ruleClass alloc] init];   //重置对象
        }
    }
}

+ (LSCExportTypeRule *)defaultRule
{
    @synchronized (self) {
        
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            
            if (!_defaultRule)
            {
                if (!_ruleClass)
                {
                    _ruleClass = [LSCExportTypeRule class];
                }
                _defaultRule = [[_ruleClass alloc] init];
            }
            
        });
        
        return _defaultRule;
    }
}

- (void)addExportFilter:(id<LSCExportFilter>)filter
{
    [self.filters insertObject:filter atIndex:0];
}

- (BOOL)filterClassMethod:(const Method)method
                 selector:(SEL)selector
                    class:(nonnull Class)cls
{
    __block BOOL result = NO;
    [self.filters enumerateObjectsUsingBlock:^(id<LSCExportFilter>  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        
        if ([obj respondsToSelector:@selector(filterClassMethod:selector:class:)])
        {
            if ([obj filterClassMethod:method selector:selector class:cls])
            {
                result = YES;
                *stop = YES;
            }
        }
    
    }];
    
    return result;
}

- (BOOL)filterInstanceMethod:(const Method)method
                    selector:(SEL)selector
                       class:(nonnull Class)cls
{
    __block BOOL result = NO;
    [self.filters enumerateObjectsUsingBlock:^(id<LSCExportFilter>  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        
        if ([obj respondsToSelector:@selector(filterInstanceMethod:selector:class:)])
        {
            if ([obj filterInstanceMethod:method selector:selector class:cls])
            {
                result = YES;
                *stop = YES;
            }
        }
        
    }];
    
    return result;
}

- (BOOL)filterProperty:(const objc_property_t)prop class:(Class)cls
{
    __block BOOL result = NO;
    [self.filters enumerateObjectsUsingBlock:^(id<LSCExportFilter>  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        
        if ([obj respondsToSelector:@selector(filterProperty:class:)])
        {
            if ([obj filterProperty:prop class:cls])
            {
                result = YES;
                *stop = YES;
            }
        }
        
    }];
    
    return result;
}

- (NSString *)typeNameByClass:(Class)aClass
{
    return [NSStringFromClass(aClass) stringByReplacingOccurrencesOfString:@"." withString:@"_"];
}

- (NSString *)prototypeNameByTypeName:(NSString *)typeName
{
    return [NSString stringWithFormat:@"_%@_PROTOTYPE_", typeName];
}

- (NSString *)constructMethodName
{
    return @"init";
}

- (NSString *)methodNameBySelector:(SEL)selector
{
    NSString *name = NSStringFromSelector(selector);
    
    if ([name hasPrefix:@"init"])
    {
        //检测是否为初始化方法
        if (name.length > 4)
        {
            unichar ch = [name characterAtIndex:4];
            //如果第五个字符为大写，则认为该方法为初始化方法
            if (ch >= 'A' && ch <= 'Z')
            {
                return self.constructMethodName;
            }
        }
        else
        {
            return self.constructMethodName;
        }
    }
    
    //只取Selector第一部分名称作为方法名
    return [name componentsSeparatedByString:@":"][0];
}

#pragma mark - Private

- (instancetype)init
{
    if (self = [super init])
    {
        self.filters = [NSMutableArray array];
        
        //添加默认导出过滤器
        LSCDefaultExportFilter *defaultExportFilter = [[LSCDefaultExportFilter alloc] init];
        [self addExportFilter:defaultExportFilter];
    }
    return self;
}

@end
