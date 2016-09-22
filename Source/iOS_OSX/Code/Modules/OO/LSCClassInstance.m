//
//  LSCClassInstance.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/9/19.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "LSCClassInstance.h"
#import "LSCClassInstance_Private.h"
#import "LSCValue_Private.h"

@implementation LSCClassInstance

- (instancetype)initWithState:(lua_State *)state atIndex:(int)index
{
    if (self = [super init])
    {
        int top = lua_gettop(state);
        if (index < 0)
        {
            //转换索引为正数，方便在调用调用属性或方法时出现指向不正确问题
            index = top + index + 1;
        }
        
        if (top < index)
        {
            return nil;
        }
        
        self.state = state;
        self.index = index;
        
        if (lua_istable(state, index))
        {
            lua_pushvalue(state, index);
            
            lua_pushstring(state, "_nativeClassName");
            lua_gettable(state, -2);
            if (lua_isstring(state, -1))
            {
                NSString *className = [NSString stringWithUTF8String:lua_tostring(state, -1)];
                self.ownerClass = NSClassFromString(className);
            }
            else
            {
                lua_settop(state, -3);
                return nil;
            }
            
            lua_settop(state, -2);
            
            lua_pushstring(state, "_nativeObject");
            lua_gettable(state, -2);
            if (lua_islightuserdata(state, -1))
            {
                self.nativeObject = (__bridge LSCObjectClass *)(lua_topointer(state, -1));
            }
            else
            {
                lua_settop(state, -3);
                return nil;
            }
            
            lua_settop(state, -3);
        }
        else
        {
            return nil;
        }
        
    }
    return self;
}

- (void)setField:(LSCValue *)value forName:(NSString *)name
{
    lua_pushvalue(self.state, self.index);
    
    [value pushWithState:self.state];
    lua_setfield(self.state, -2, [name UTF8String]);
    
    lua_settop(self.state, -2);
}

- (LSCValue *)getFieldForName:(NSString *)name
{
    lua_pushvalue(self.state, self.index);
    
    lua_pushstring(self.state, [name UTF8String]);
    lua_gettable(self.state, -2);
    
    LSCValue *retValue = [LSCValue valueWithState:self.state atIndex:-1];
    
    lua_settop(self.state, -3);
    
    return retValue;
}

- (LSCValue *)callMethodWithName:(NSString *)name arguments:(NSArray *)arguments
{
    lua_pushvalue(self.state, self.index);

    LSCValue *resultValue = nil;

    lua_getfield(self.state, -1, [name UTF8String]);
    if (lua_isfunction(self.state, -1))
    {
        //如果为function则进行调用
        //第一个参数为实例引用
        lua_pushvalue(self.state, self.index);
        
        __weak LSCClassInstance *theInstance = self;
        [arguments enumerateObjectsUsingBlock:^(LSCValue *_Nonnull value, NSUInteger idx, BOOL *_Nonnull stop) {
            
            [value pushWithState:theInstance.state];
            
        }];
        
        if (lua_pcall(self.state, (int)arguments.count + 1, 1, 0) == 0)
        {
            //调用成功
            resultValue = [LSCValue valueWithState:self.state atIndex:-1];
        }
    }
    
    lua_settop(self.state, -3);
    
    return resultValue;
}

@end
