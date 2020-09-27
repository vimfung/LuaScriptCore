//
//  LuaTestType.h
//  LuaScriptCoreTests
//
//  Created by 冯鸿杰 on 2020/9/4.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LuaScriptCore.h"
#import "LSCExportTypeModuleHeader.h"

NS_ASSUME_NONNULL_BEGIN

@interface LuaTestType : NSObject <LSCExportType>

@property (nonatomic, strong) NSString *A;

@property (nonatomic, strong) NSArray *nameArr;

+ (void)callMethod;

+ (NSInteger)callMethodIntValue:(NSInteger)value;

+ (double)callMethod:(id)obj doubleValue:(double)value;

+ (BOOL)callMethod:(id)obj booleanValue:(BOOL)value;

+ (NSString *)callMethod:(id)obj stringValue:(NSString *)stringValue;

@end

NS_ASSUME_NONNULL_END
