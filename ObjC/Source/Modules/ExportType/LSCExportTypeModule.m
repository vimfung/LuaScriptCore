//
//  LSCExportTypeModule.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2020/9/2.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LSCExportTypeModule.h"
#import "LSCContext+ExportType.h"
#import "LSCApiAdapter+ExportType.h"
#import "LSCValue.h"
#import "LSCTypeValue.h"
#import "LSCInstanceValue.h"

@implementation LSCExportTypeModule

#pragma mark - LSCModule

+ (NSString *)moduleId
{
    return @"cn.vimfung.luascriptcore.module.ExportType";
}

+ (void)registerModuleWithContext:(LSCContext *)context
{
    [LSCValue registerValueType:[LSCTypeValue class]];
    [LSCValue registerValueType:[LSCInstanceValue class]];
    
    context.typeDescriptionMap = [NSMutableDictionary dictionary];
    
    LSCApiAdapter *apiAdapter = [LSCApiAdapter defaultApiAdapter];
    [apiAdapter watchGlobalWithContext:context];
}

@end
