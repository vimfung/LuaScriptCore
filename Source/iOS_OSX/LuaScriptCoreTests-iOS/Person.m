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
#import "NativePerson.h"

static LSCValue *_func = nil;
static LSCManagedValue *_managedFunc = nil;

@implementation Person

- (instancetype)init
{
    if (self = [super init])
    {
        self.name = @"vimfung";
    }
    return self;
}

- (void)setName:(NSString *)name
{
    _name = name;
}

- (void)speak:(NSString *)content
{
    NSLog(@"%@ speak:\"%@\"", self.name, content);
}

- (void)speakWithAge:(BOOL)age
{
    NSLog(@"%@ %ld years old", self.name, (long)age);
}

- (void)speakWithPos:(int)pos
{
    NSLog(@"%@ pos = %d", self.name, pos);
}

- (void)dealloc
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSLog(@"person dealloc");
    });
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

+ (Person *)createPersonError
{
    [Env.defaultContext raiseExceptionWithMessage:@"can't create person"];
    return [self createPerson];
}

+ (NativePerson *)createNativePerson
{
    return [[NativePerson alloc] init];
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
    _func = nil;
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

+ (void)testFuncRelease:(LSCFunction *)func
{
    
}

+ (BOOL)returnBoolean
{
    return NO;
}

+ (char)returnChar
{
    return 0;
}

@end
