//
//  LSCTPerson.h
//  Sample
//
//  Created by 冯鸿杰 on 16/9/22.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LuaScriptCore.h"

@interface LSCTPerson : NSObject <LSCExportType>

@property (nonatomic, copy) NSString *name;

- (void)speak;

- (void)walk;

@end
