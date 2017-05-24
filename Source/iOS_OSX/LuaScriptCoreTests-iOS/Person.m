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
#import "LSCManagedValue.h"

static LSCValue *_func = nil;
static LSCManagedValue *_managedFunc = nil;

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

+ (void)retainHandler2:(LSCFunction *)handler
{
    _managedFunc = [[LSCManagedValue alloc] initWithValue:[LSCValue functionValue:handler] context:[Env defaultContext]];
}

+ (void)releaseHandler
{
    [[Env defaultContext] releaseValue:_func];
}

+ (void)releaseHandler2
{
    _managedFunc = nil;
}

+ (void)callHandler
{
    if (_func)
    {
        [[_func toFunction] invokeWithArguments:nil];
    }
}

+ (void)callHandler2
{
    if (_managedFunc)
    {
        [[_managedFunc.source toFunction] invokeWithArguments:nil];
    }
}

@end
