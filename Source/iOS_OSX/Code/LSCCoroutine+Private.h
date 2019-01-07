//
//  LSCCoroutine+Private.h
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2019/1/1.
//  Copyright © 2019年 vimfung. All rights reserved.
//

#import "LSCCoroutine.h"
#import "LSCEngineAdapter.h"

NS_ASSUME_NONNULL_BEGIN

@interface LSCCoroutine ()

/**
 状态对象
 */
@property (nonatomic) lua_State *state;

/**
 关联ID
 */
@property (nonatomic, copy) NSString *linkId;

@end

NS_ASSUME_NONNULL_END
