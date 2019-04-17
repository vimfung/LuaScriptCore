//
//  Env.h
//  Sample
//
//  Created by 冯鸿杰 on 2019/3/11.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "LuaScriptCore.h"

NS_ASSUME_NONNULL_BEGIN

@interface Env : NSObject

+ (LSCContext *)defaultContext;

+ (LSCScriptController *)runScriptConfig;

@end

NS_ASSUME_NONNULL_END
