//
//  LuaScriptCoreTests_OSX.m
//  LuaScriptCoreTests-OSX
//
//  Created by 冯鸿杰 on 16/11/14.
//  Copyright © 2016年 vimfung. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "LuaScriptCore.h"
#import "TestModule.h"
#import "Person.h"
#import "NativePerson.h"
#import "Env.h"

@interface LuaScriptCoreTests_OSX : XCTestCase

@property (nonatomic, strong) LSCContext *context;

@end

@implementation LuaScriptCoreTests_OSX

- (void)setUp
{
    [super setUp];
    self.context = [Env defaultContext];
    [self.context onException:^(NSString *message) {
       
        NSLog(@"lua exception = %@", message);
        
    }];
}

- (void)tearDown
{
    [super tearDown];
    self.context = nil;
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
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_OSX class]];
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
    LSCValue *resValue = [self.context evalScriptFromString:@"return TestModule:test();"];
    XCTAssertTrue([[resValue toString] isEqualToString:@"Hello World!"], "result value is not equal 'Hello World!'");
}

- (void)testRegisterClass
{
    [self.context evalScriptFromString:@"local p = Person:createPerson(); print(p); p.name = 'vim'; Person:printPersonName(p); p:speak('Hello World!');"];
}

- (void)testCreateObjectWithParams
{
    [self.context evalScriptFromString:@"function Person.prototype:init(value) print(value); end local p = Person('xxx'); print(p);"];
}

- (void)testClassModuleName
{
    [self.context evalScriptFromString:@"print(Person.name);"];
}

- (void)testClassIsSubclassOf
{
    [self.context evalScriptFromString:@"print(Person:subclassOf(Object));"];
}

- (void)testClassIsInstanceOf
{
    [self.context evalScriptFromString:@"local p = Person(); print(p:instanceOf(Person)); print(p:instanceOf(Object));"];
}

- (void)testClassMethodInherited
{
    [self.context evalScriptFromString:@"print(Object); Person:subclass('ChinesePerson'); function Person.prototype:init () print(self.super); print ('Person init') end function ChinesePerson.prototype:init () self.super.init(self); print ('Chinese init'); end local c = ChinesePerson(); print(c); local p = Person(); print(p);"];
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
    [self.context registerMethodWithName:@"testTuple" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
        
        LSCTuple *tuple = [[LSCTuple alloc] init];
        [tuple addReturnValue:@1000];
        [tuple addReturnValue:@24];
        
        return [LSCValue tupleValue:tuple];
        
    }];
    
    [self.context evalScriptFromString:@"local a,b = testTuple(); print(a, b);"];
}

- (void)testModuleTupleReturnValue
{
    [self.context evalScriptFromString:@"local a,b = TestModule:testTuple(); print(a, b);"];
}

- (void)testClassInstanceTupleReturnValue
{
    [self.context evalScriptFromString:@"local p = Person(); local a,b = p:test(); print(a, b);"];
}

- (void)testFunctionInvokeReturn
{
    [self.context registerMethodWithName:@"test" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
        
        LSCFunction *func = [arguments[0] toFunction];
        LSCValue *retValue = [func invokeWithArguments:nil];
        NSLog(@"retValue = %@", retValue);
        
        return nil;
        
    }];
    
    [self.context evalScriptFromString:@"test(function() print('test 1'); return 1,'Hello World',3; end); test(function() print('test 2'); return 'xxxx'; end); test(function() print('test 3'); end);"];
}

- (void)testObjProxy
{
    [self.context evalScriptFromString:@"print(Person); local p = Person:createNativePerson(); print(p); p.name = 'vim'; p:speak('Hello World!');"];
}

- (void)testClassImportAndObjectClass
{
    [self.context evalScriptFromString:@"print(Person, NativePerson); local p = NativePerson:createPerson(); print(p); p.name = 'abc'; p:speak('Hello World!');"];
}

- (void)testRetainRelease
{
    [self.context evalScriptFromString:@"local test = function() print('test func') end; test(); Person:retainHandler(test);"];
    [self.context evalScriptFromString:@"print('-------------1'); Person:callHandler(); Person:releaseHandler();"];
    [self.context evalScriptFromString:@"print('-------------2'); Person:callHandler();"];
}

- (void)testRetainRelease_2
{
    [self.context evalScriptFromString:@"function getFunc() return function (a, b) print(\"------\", a, b); return a * b; end end"];
    LSCValue *funcValue = [self.context callMethodWithName:@"getFunc" arguments:nil];
    
    LSCValue *retValue = [[funcValue toFunction] invokeWithArguments:@[[LSCValue numberValue:@100], [LSCValue numberValue:@924]]];
    NSLog(@"%@", retValue);
}

- (void)testRetainRelease_3
{
    [self.context evalScriptFromString:@"local test = function() print('test func') end; test(); Person:retainHandler2(test);"];
    [self.context evalScriptFromString:@"print('-------------1'); Person:callHandler2(); Person:releaseHandler2();"];
    [self.context evalScriptFromString:@"print('-------------2'); Person:callHandler2();"];
}

- (void)testCoroutine
{
    [self.context registerMethodWithName:@"GetValue" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
        
        LSCValue *value = [LSCValue numberValue:@1024];
        return value;
        
    }];
    
    [self.context registerMethodWithName:@"GetPixel" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
        
        LSCTuple * tuple = [[LSCTuple alloc] init];
        [tuple addReturnValue:[LSCValue numberValue:@100]];
        [tuple addReturnValue:[LSCValue numberValue:@20]];
        [tuple addReturnValue:[LSCValue numberValue:@232]];
        
        return [LSCValue tupleValue:tuple];
        
    }];
    
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_OSX class]];
    NSString *path = [bundle pathForResource:@"coroutine" ofType:@"lua"];
    [self.context evalScriptFromFile:path];
}

- (void)testNewTypeExporter
{
    [self.context evalScriptFromString:@"print(SubLuaLog); function SubLuaLog.prototype:init () print('SubLuaLog object init'); end; local t = SubLuaLog(); print(t); t.xxx = 'aaaa'; print (t.xxx); t.name = 'vim'; t:printName();"];
}

- (void)testLuaFuncCall
{
    [self.context evalScriptFromString:@"App = {}; function App:onCreate(arg) print('Hello World', arg); end"];
//    [self.context evalScriptFromString:@"App:onCreate();"];
    LSCValue *value = [self.context evalScriptFromString:@"return App"];
    LSCFunction *func = [[self.context evalScriptFromString:@"return App.onCreate"] toFunction];
    [func invokeWithArguments:@[value, [LSCValue integerValue:1024]]];
}

- (void)testMultiThreadCall
{
    XCTestExpectation *exp = [[XCTestExpectation alloc] initWithDescription:@""];
    
    dispatch_group_t group = dispatch_group_create();
    
    for (int i = 0; i < 100; i++)
    {
        dispatch_group_enter(group);
        dispatch_group_async(group, dispatch_get_global_queue(0, 0), ^{
            
            [self.context evalScriptFromString:@"Person.prototype.age = {set = function(self, value) print('set value = ', value); self._value = value end, get = function(self) print('get value = ', self._value); return self._value end}; local p = Person:createPerson(); p.age = 20; print('p.age = ', p.age);"];
            
            dispatch_group_leave(group);
            
        });
    }
    
    dispatch_group_notify(group, dispatch_get_main_queue(), ^{
        
        [exp fulfill];
        
    });
    
    [self waitForExpectations:@[exp] timeout:20];
}

@end
