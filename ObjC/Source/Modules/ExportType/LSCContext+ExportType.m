//
//  LSCContext+ExportType.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/2.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCContext+ExportType.h"
#import "LSCTypeDescription+Private.h"
#import "LSCExportTypeModule.h"
#import "LSCExportTypeRule.h"
#import <objc/runtime.h>

static NSString *const typeDescriptionMapKey = @"_typeDescriptionMap";

@implementation LSCContext (ExportType)

- (NSMutableDictionary<NSString *, LSCTypeDescription *> *)typeDescriptionMap
{
    return [self getUserdataWithKey:typeDescriptionMapKey
                             module:[LSCExportTypeModule class]];
}

- (void)setTypeDescriptionMap:(NSMutableDictionary<NSString *,LSCTypeDescription *> *)typeDescriptionMap
{
    [self setUserdata:typeDescriptionMap
               forKey:typeDescriptionMapKey
               module:[LSCExportTypeModule class]];
}

- (LSCTypeDescription *)typeDescriptionByName:(NSString *)name
{
    if ([name isKindOfClass:[NSString class]])
    {
        LSCTypeDescription *typeDescriptor = self.typeDescriptionMap[name];
        if (!typeDescriptor)
        {
            if ([name isEqualToString:[LSCTypeDescription objectTypeDescription].typeName])
            {
                //为Object类
                typeDescriptor = [LSCTypeDescription objectTypeDescription];
            }
            else
            {
                //如果映射表中不存在类型，则需要在原生层的类型中进行查找
                typeDescriptor = [self _findExportTypeWithName:name];
            }
            
            if (typeDescriptor)
            {
                //设置到类型映射表中
                [self.typeDescriptionMap setObject:typeDescriptor forKey:name];
            }
        }
        
        return typeDescriptor;
    }
    
    return nil;
}

/**
 根据类型名称查找类型
 
 @param typeName 类型名称
 @return 类型描述
 */
- (LSCTypeDescription *)_findExportTypeWithName:(NSString *)typeName
{
    __block NSString *name = typeName;
    __block Class cls = NSClassFromString(name);
    
    if (cls == NULL)
    {
        //转换类型名称，有可能传入"名称空间_类型名称"格式的类型
        NSString *targetName = [typeName stringByReplacingOccurrencesOfString:@"_" withString:@"."];
        cls = NSClassFromString(targetName);
        if (cls != NULL)
        {
            name = targetName;
        }
    }
    
    if (cls == NULL)
    {
        //由于Swift的类型名称为“模块名称.类型名称”，因此尝试拼接模块名称后进行类型检测
        NSMutableArray<NSBundle *> *allBundles = [NSMutableArray array];
        if ([NSBundle allBundles].count > 0)
        {
            [allBundles addObjectsFromArray:[NSBundle allBundles]];
        }
        if ([NSBundle allFrameworks].count > 0)
        {
            [allBundles addObjectsFromArray:[NSBundle allFrameworks]];
        }
        
        [allBundles enumerateObjectsUsingBlock:^(NSBundle * _Nonnull bundle, NSUInteger idx, BOOL * _Nonnull stop) {
            
            NSString *moduleName = bundle.executablePath.lastPathComponent;
            moduleName = [moduleName stringByReplacingOccurrencesOfString:@"-" withString:@"_"];
            
            if (moduleName)
            {
                NSString *targetName = [NSString stringWithFormat:@"%@.%@", moduleName, typeName];
                cls = NSClassFromString(targetName);
                if (cls != NULL)
                {
                    name = targetName;
                    *stop = YES;
                }
            }
            
        }];
    }
    
    if (cls != NULL)
    {
        LSCTypeDescription *typeDescriptor = [LSCTypeDescription typeDescriptionByClass:cls];
        if (typeDescriptor)
        {
            [self.typeDescriptionMap setObject:typeDescriptor forKey:typeName];
            
            //检测父类
            Class parentCls = class_getSuperclass(cls);
            NSString *parentName = [[LSCExportTypeRule defaultRule] typeNameByClass:parentCls];
            LSCTypeDescription *parentTypeDescription = [self typeDescriptionByName:parentName];
            if (!parentTypeDescription)
            {
                parentTypeDescription = [LSCTypeDescription objectTypeDescription];
            }
            
            typeDescriptor.parentTypeDescription = parentTypeDescription;
        }
        
        return typeDescriptor;
    }
    
    return nil;
}

@end
