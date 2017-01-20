//
//  LuaScriptCoreTests_iOS.m
//  LuaScriptCoreTests-iOS
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "LuaScriptCore.h"
#import "TestModule.h"
#import "Person.h"
#import "LSCTuple.h"

@interface LuaScriptCoreTests_iOS : XCTestCase

@property (nonatomic, strong) LSCContext *context;

@end

@implementation LuaScriptCoreTests_iOS

- (void)setUp
{
    [super setUp];
    
    self.context = [[LSCContext alloc] init];
    [self.context onException:^(NSString *message) {
       
        NSLog(@"error = %@", message);
        
    }];
}

- (void)testEvalScript
{
    LSCValue *value = [self.context evalScriptFromString:@"return 4 * 256; "];
    XCTAssertNotNil(value, "value is nil");
    NSLog(@"retValue = %@", value);
    
    value = [self.context evalScriptFromString:@"return 4 * 256, 'aa', 'bb'; "];
    XCTAssertNotNil(value, "value is nil");
    NSLog(@"retValue = %@", value);
}

- (void)testEvalScriptFromFile
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    NSString *path = [bundle pathForResource:@"Test" ofType:@"lua"];
    XCTAssertNotNil(path, "path is nil");
    LSCValue *value = [self.context evalScriptFromFile:path];
    XCTAssertNotNil(value, "value is nil");
    NSLog(@"retValue = %@", value);
}

- (void)testRegisterMethodAndCall
{
    [self.context registerMethodWithName:@"add" block:^LSCValue *(NSArray *arguments) {
        
        LSCValue *v1 = arguments[0];
        LSCValue *v2 = arguments[1];
        
        return [LSCValue integerValue:[v1 toInteger] + [v2 toInteger]];
        
    }];
    
    [self.context registerMethodWithName:@"returnTuple" block:^LSCValue *(NSArray *arguments) {
        
        LSCValue *v1 = arguments[0];
        LSCValue *v2 = arguments[1];
        LSCTuple *tuple = [[LSCTuple alloc] init];
        [tuple addReturnValue:[v1 toObject]];
        [tuple addReturnValue:[v2 toObject]];
        
        return [LSCValue tupleValue:tuple];
        
    }];
    
    NSInteger v1 = 30;
    NSInteger v2 = 55;
    LSCValue *resValue = [self.context callMethodWithName:@"add" arguments:@[[LSCValue integerValue:v1], [LSCValue integerValue:v2]]];
    XCTAssertNotNil(resValue, "result value is nil");
    NSLog(@"resValue = %@", resValue);
    
    resValue = [self.context callMethodWithName:@"returnTuple" arguments:@[[LSCValue integerValue:v1], [LSCValue integerValue:v2]]];
    XCTAssertNotNil(resValue, "result value is nil");
    NSLog(@"resValue = %@", resValue);
    
    
}

- (void)testRegisterModule
{
    [self.context registerModuleWithClass:[TestModule class]];
    LSCValue *resValue = [self.context evalScriptFromString:@"return TestModule.test();"];
    XCTAssertTrue([[resValue toString] isEqualToString:@"Hello World!"], "result value is not equal 'Hello World!'");
}

- (void)testRegisterClass
{
    [self.context registerModuleWithClass:[Person class]];
    [self.context evalScriptFromString:@"local p = Person.createPerson(); p:setName('vim'); Person.printPersonName(p); p:speak('Hello World!');"];
}

- (void)testCreateObjectWithParams
{
    [self.context registerModuleWithClass:[Person class]];
    [self.context evalScriptFromString:@"function Person:init(value) print(value); end local p = Person.create(); print(p);"];
}

- (void)testClassModuleName
{
    [self.context registerModuleWithClass:[Person class]];
    [self.context evalScriptFromString:@"print(Person.name);"];
}

- (void)testClassIsSubclassOf
{
    [self.context registerModuleWithClass:[Person class]];
    [self.context evalScriptFromString:@"print(Person.subclassOf(Object));"];
}

- (void)testClassIsInstanceOf
{
    [self.context registerModuleWithClass:[Person class]];
    [self.context evalScriptFromString:@"local p = Person.create(); print(p:instanceOf(Person)); print(p:instanceOf(Object));"];
}

- (void)testClassMethodInherited
{
    [self.context registerModuleWithClass:[Person class]];
    [self.context evalScriptFromString:@"Person.subclass('Chinese'); function Person:init () print ('Person init') end function Chinese:init () self.super.init(self); print ('Chinese init'); end local c = Chinese.create();"];
}

- (void)testSetGlobalVar
{
    [self.context setGlobalWithValue:[LSCValue objectValue:@(1)] forName:@"test"];
    [self.context evalScriptFromString:@"print(test);"];
    
    LSCValue *value = [self.context getGlobalForName:@"test"];
    NSLog(@"test = %@", value);
}

- (void)testTupleReturnValue
{
    [self.context registerMethodWithName:@"test" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
       
        LSCTuple *tuple = [[LSCTuple alloc] init];
        [tuple addReturnValue:@1000];
        [tuple addReturnValue:@24];
        
        return [LSCValue tupleValue:tuple];
        
    }];
    
    [self.context evalScriptFromString:@"local a,b = test(); print(a, b);"];
}

- (void)testModuleTupleReturnValue
{
    [self.context registerModuleWithClass:[TestModule class]];
    [self.context evalScriptFromString:@"local a,b = TestModule.testTuple(); print(a, b);"];
}

- (void)testClassInstanceTupleReturnValue
{
    [self.context registerModuleWithClass:[Person class]];
    [self.context evalScriptFromString:@"local p = Person.create(); local a,b = p:test(); print(a, b);"];
}

- (void)testFunctionInvokeReturn
{
    [self.context registerMethodWithName:@"test" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
       
        LSCFunction *func = [arguments[0] toFunction];
        LSCValue *retValue = [func invokeWithArguments:nil];
        NSLog(@"retValue = %@", retValue);
        
        return nil;
        
    }];
    
    [self.context evalScriptFromString:@"test(function() return 1,'Hello World',3; end); test(function() return 'xxxx'; end); test(function() end);"];
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
    self.context = nil;
}

@end
