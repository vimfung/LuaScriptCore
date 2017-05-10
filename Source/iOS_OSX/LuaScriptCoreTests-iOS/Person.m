//
//  Person.m
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import "Person.h"
#import "LSCTuple.h"
#import "LSCFunction.h"
#import "LSCValue.h"
#import "Env.h"

static LSCValue *_func = nil;

@implementation Person

- (void)speak:(NSString *)content
{
    NSLog(@"%@ speak:\"%@\"", self.name, content);
}

- (LSCTuple *)test
{
    LSCTuple *tuple = [[LSCTuple alloc] init];
    [tuple addReturnValue:@"Hello"];
    [tuple addReturnValue:@"World"];
    
    return tuple;
}

+ (void)printPersonName:(Person *)person
{
    NSLog(@"person name = %@", person.name);
}

+ (Person *)createPerson
{
    return [[Person alloc] init];
}

+ (void)retainHandler:(LSCFunction *)handler
{
    _func = [LSCValue functionValue:handler];
    [[Env defaultContext] retainValue:_func];
}

+ (void)releaseHandler
{
    [[Env defaultContext] releaseValue:_func];
}

+ (void)callHandler
{
    if (_func)
    {
        [[_func toFunction] invokeWithArguments:nil];
    }
}

@end
