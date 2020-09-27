//
//  LuaChildType.h
//  LuaScriptCoreTests
//
//  Created by 冯鸿杰 on 2020/9/22.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import "LuaTestType.h"

NS_ASSUME_NONNULL_BEGIN

@interface LuaChildType : LuaTestType

@property (nonatomic, copy, readonly) NSString *bStr;

@property (nonatomic, copy, getter=dStr, setter=setDStr:) NSString *cStr;

- (void)setBStr:(NSString * _Nonnull)bStr;

- (NSString *)dStr;

- (void)setDStr:(NSString *)dStr;

@end

NS_ASSUME_NONNULL_END
