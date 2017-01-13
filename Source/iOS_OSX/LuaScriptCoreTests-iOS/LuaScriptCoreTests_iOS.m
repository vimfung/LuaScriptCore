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
    LSCValue *value = [self.context evalScriptFromString:@"return 4 * 256;"];
    XCTAssertNotNil(value, "value is nil");
    XCTAssertEqual([value toInteger], 1024, "result value not equal 1024.");
}

- (void)testEvalScriptFromFile
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    NSString *path = [bundle pathForResource:@"Test" ofType:@"lua"];
    XCTAssertNotNil(path, "path is nil");
    LSCValue *value = [self.context evalScriptFromFile:path];
    XCTAssertNotNil(value, "value is nil");
    XCTAssertEqual([value toInteger], 1024, "result value not equal 1024");
}

- (void)testRegisterMethodAndCall
{
    [self.context registerMethodWithName:@"add" block:^LSCValue *(NSArray *arguments) {
        
        LSCValue *v1 = arguments[0];
        LSCValue *v2 = arguments[1];
        
        return [LSCValue integerValue:[v1 toInteger] + [v2 toInteger]];
        
    }];
    
    NSInteger v1 = 30;
    NSInteger v2 = 55;
    LSCValue *resValue = [self.context callMethodWithName:@"add" arguments:@[[LSCValue integerValue:v1], [LSCValue integerValue:v2]]];
    XCTAssertNotNil(resValue, "result value is nil");
    XCTAssertEqual([resValue toInteger], v1 + v2, "result is not equal %ld", v1 + v2);
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
    [self.context evalScriptFromString:@"print(name);"];
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

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
    self.context = nil;
}

@end
