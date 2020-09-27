//
//  LSCDefaultExportFilter.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/3.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCDefaultExportFilter.h"
#import <objc/runtime.h>

@implementation LSCDefaultExportFilter

- (BOOL)filterClassMethod:(const Method)method
                 selector:(SEL)selector
                    class:(nonnull Class)cls
{
    NSString *selectorName = NSStringFromSelector(selector);
    if ([selectorName hasPrefix:@"_"]
        || [selectorName hasPrefix:@"."])
    {
        return YES;
    }
    
    return NO;
}

- (BOOL)filterInstanceMethod:(const Method)method
                    selector:(SEL)selector
                       class:(nonnull Class)cls
{
    NSString *selectorName = NSStringFromSelector(selector);

    if ([selectorName hasPrefix:@"_"]
        || [selectorName hasPrefix:@"."])
    {
        return YES;
    }
    
    //-2 self selector
    int argCount = method_getNumberOfArguments(method) - 2;
    if (argCount <= 1)
    {
        //参数数量小于或等于1的情况下可能是属性selector，需要进一步判断
        BOOL isPropSel = [LSCDefaultExportFilter _isPropertySelector:selector forClass:cls];
        if (isPropSel)
        {
            return YES;
        }
    }
    
    return NO;
}

- (BOOL)filterProperty:(const objc_property_t)prop class:(Class)cls
{
    NSString *propertyName = [NSString stringWithUTF8String:property_getName(prop)];
    
    if ([propertyName hasPrefix:@"_"]
        || [propertyName isEqualToString:@"hash"]
        || [propertyName isEqualToString:@"superclass"]
        || [propertyName isEqualToString:@"description"]
        || [propertyName isEqualToString:@"debugDescription"])
    {
        return YES;
    }
    
    return NO;
}

#pragma mark - Private


/**
 获取属性选择子映射表

 @return 数据表
 */
+ (NSMutableDictionary<NSString *, NSMutableDictionary<NSString *, NSString *> *> *)_propertySelectorsMap
{
    static NSMutableDictionary *propSelectorMap = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        propSelectorMap = [NSMutableDictionary dictionary];
    });
    
    return propSelectorMap;
}

/**
 获取指定类型的属性选择子列表

 @param cls 类型
 @return 选择子列表
 */
+ (NSDictionary<NSString *, NSString *> *)_propSelectorsByClass:(Class)cls
{
    NSString *clsName = NSStringFromClass(cls);
    
    NSMutableDictionary *map = [self _propertySelectorsMap];
    NSMutableDictionary *selectors = map[clsName];
    if (!selectors)
    {
        selectors = [NSMutableDictionary dictionary];
        [map setObject:selectors forKey:clsName];
        
        //生成selector列表
        uint count = 0;
        objc_property_t *properties = class_copyPropertyList(cls, &count);
        for (int i = 0; i < count; i++)
        {
            objc_property_t property = *(properties + i);
            NSString *propertyName = [NSString stringWithUTF8String:property_getName(property)];
            
            //获取属性特性
            BOOL readonly = NO;
            NSString *getterName = nil;
            NSString *setterName = nil;
            uint attrCount = 0;
            objc_property_attribute_t *attrs = property_copyAttributeList(property, &attrCount);
            for (int j = 0; j < attrCount; j++)
            {
                objc_property_attribute_t attr = *(attrs + j);
                if (strcmp(attr.name, "G") == 0)
                {
                    getterName = [NSString stringWithUTF8String:attr.value];
                }
                else if (strcmp(attr.name, "S") == 0)
                {
                    //Setter
                    setterName = [NSString stringWithUTF8String:attr.value];
                }
                else if (strcmp(attr.name, "R") == 0)
                {
                    //只读属性
                    readonly = YES;
                }
            }
            free(attrs);
            
            //没有获得getter和setter名字时，则表示使用的是默认名称
            if (!getterName)
            {
                getterName = propertyName;
            }
            if (!setterName)
            {
                setterName = [NSString stringWithFormat:@"set%@%@:",
                              [propertyName.capitalizedString substringToIndex:1],
                              [propertyName substringFromIndex:1]];
            }
            if (readonly)
            {
                setterName = nil;
            }
            
            //写入选择子列表
            if (getterName)
            {
                [selectors setObject:propertyName forKey:getterName];
            }
            if (setterName)
            {
                [selectors setObject:propertyName forKey:setterName];
            }
        }
        free(properties);
    }
    
    return selectors;
}


/**
 是否为属性选择子

 @param selector 选择子
 @param cls 类型
 @return YES 为属性选择子，否则不是
 */
+ (BOOL)_isPropertySelector:(SEL)selector forClass:(Class)cls
{
    BOOL hasExists = NO;
    NSString *selName = NSStringFromSelector(selector);
    
    while (1)
    {
        NSDictionary *propSelectors = [self _propSelectorsByClass:cls];
        if (propSelectors[selName])
        {
            //为属性selector
            hasExists = YES;
            break;
        }
        
        if (cls == [NSObject class])
        {
            break;
        }
        
        cls = class_getSuperclass(cls);
    }
    
    return hasExists;
}

@end
