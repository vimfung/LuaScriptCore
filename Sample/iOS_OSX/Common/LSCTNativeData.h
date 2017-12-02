//
//  LSCTNativeData.h
//  Sample
//
//  Created by 冯鸿杰 on 2017/4/17.
//  Copyright © 2017年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LuaScriptCore.h"

@interface LSCTNativeData : NSObject <LSCExportType>

@property (nonatomic, copy) NSString *dataId;

+ (LSCTNativeData *)createData;

- (void)setData:(NSString *)data key:(NSString *)key;

- (NSString *)getData:(NSString *)key;

@end
