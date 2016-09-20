//
//  TestModule.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCModule.h"
#import <UIKit/UIKit.h>

@interface TestModule : LSCModule

@property (nonatomic, copy) NSString *fieldName;


- (void)test;

- (int)callWithName:(NSString *)name byIndex:(NSInteger)index;

@end
