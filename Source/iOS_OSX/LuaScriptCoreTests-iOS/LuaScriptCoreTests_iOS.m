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
#import "Chinese.h"
#import "English.h"
#import "America.h"
#import "NativePerson.h"
#import "LSCTuple.h"
#import "Env.h"
#import "SubLuaLog.h"
#import "LSCOperationQueue.h"
#import "LSCThread.h"
#import <objc/message.h>

#import "LSCEngineAdapter.h"

@interface LuaScriptCoreTests_iOS : XCTestCase

@property (nonatomic, strong) LSCContext *context;

@end

@implementation LuaScriptCoreTests_iOS

- (void)setUp
{
    [super setUp];
    
    self.context = [Env defaultContext];
    [self.context onException:^(NSString *message) {
       
        NSLog(@"error = %@", message);
        
    }];
}

- (void)testThread
{
    XCTestExpectation *exp = [[XCTestExpectation alloc] initWithDescription:@""];
    
    LSCThread *t = [[LSCThread alloc] initWithContext:self.context handler:^LSCValue *(NSArray<LSCValue *> *arguments) {
        
        NSLog(@"arguments = %@", arguments);
        [exp fulfill];
        
        return nil;
    }];
    
    [t resumeWithArguments:@[[LSCValue integerValue:1024]]];
    
    [self waitForExpectations:@[exp] timeout:3000];
}

/**
 预期结果
 vimfung
 xxxxxxx
 */
- (void)testCreateObj
{
    [self.context evalScriptFromString:@"local p = Person(); print(p.name); local p2 = Person(\"xxxxxxx\", 18); print(p2.name);"];
}


/**
 预期结果
 set value =     20
 get value =     20
 p.age =     20.0
 person dealloc
 */
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


/**
 预期结果:
 Hello World
 */
- (void)testCompiledCode
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    [self.context evalScriptFromFile:[bundle pathForResource:@"test" ofType:@"out"]];
}


/**
 预期结果:
 */
- (void)testThreadInitContext
{
    XCTestExpectation *exp = [[XCTestExpectation alloc] initWithDescription:@""];
    
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
       
        LSCContext *context = [[LSCContext alloc] init];
        XCTAssertNotNil(context, "context is nil");
        [exp fulfill];
        
    });
    
    [self waitForExpectations:@[exp] timeout:5];
}


/**
 预期结果:
 false
 */
- (void)testReturnBoolean
{
    [self.context evalScriptFromString:@"if Person:returnBoolean() then print('true'); else print('false'); end"];
}


/**
 预期结果:
 1
 */
- (void)testReturnChar
{
    [self.context evalScriptFromString:@"print (Person:returnChar() + 1);"];
}


/**
 预期结果:
 */
- (void)testLuaFunctionRelease
{
    [self.context evalScriptFromString:@"function abc() print('---------'); end Person:testFuncRelease(abc);"];
}


/**
 预期结果:
 [Object object<0x7fd507528ac8>]
 [Person object<0x7fd507626f48>]
 */
- (void)testConstructInstance1
{
    [self.context evalScriptFromString:@"local obj = Object(); print(obj); local p = Person(); print(p);"];
}


/**
 预期结果:
 Object instance create!
 [Object object<0x7fef62321888>]
 */
- (void)testConstructInstance2
{
    [self.context evalScriptFromString:@"function Object.prototype:init() print('Object instance create!'); end local obj = Object(); print(obj);"];
}


/**
 预期结果:
 vim instance create!
 [Object object<0x7fb816830328>]
 */
- (void)testConstructInstance3
{
    [self.context evalScriptFromString:@"function Object.prototype:init(name) print(name..' instance create!'); end local obj = Object('vim'); print(obj);"];
}


/**
 预期结果:
 16.0
 [Object object]
 [Person object]
 vim
 */
- (void)testLuaScript
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    [self.context evalScriptFromFile:[bundle pathForResource:@"class" ofType:@"lua"]];
}


/**
 预期结果
 intValue    111
 */
- (void)testCustomProperty
{
    [self.context evalScriptFromString:@"local p = Person:createPerson(); p.intValue = 111; print('intValue', p.intValue);"];
}


/**
 预期结果
 error = [string "local p = Person:createPersonError(); print(p..."]:1: can't create person
 */
- (void)testRaiseException
{
    [self.context evalScriptFromString:@"local p = Person:createPersonError(); print(p);"];
}


/**
 预期结果
 set value =     20
 get value =     20
 p.age =     20.0
 */
- (void)testCustomProperty2
{
    [self.context evalScriptFromString:@"Person.prototype.age = {set = function(self, value) print('set value = ', value); self._value = value end, get = function(self) print('get value = ', self._value); return self._value end}; local p = Person:createPerson(); p.age = 20; print('p.age = ', p.age);"];
}


/**
 预期结果：
 [Person type]
 [Person object<0x7ff050d9c378>]
 vimfung
 table: 0x7ff050d9ca10
 test value
 2222
 
 {
 a = 2;
 } speak:"Hello"
 */
- (void)testGlobalVar
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    NSString *path = [bundle pathForResource:@"Test1" ofType:@"lua"];
    [self.context evalScriptFromFile:path];
}


/**
 预期结果：
 retValue = 1024
 */
- (void)testIndependentContext
{
    LSCContext *context = [[LSCContext alloc] init];
    LSCValue *value = [context evalScriptFromString:@"return 4 * 256; "];
    XCTAssertNotNil(value, "value is nil");
    NSLog(@"retValue = %@", value);
}


/**
 预期结果:
 retValue = 1024
 retValue = (
    1024,
    aa,
    bb
 )
 */
- (void)testEvalScript
{
    LSCValue *value = [self.context evalScriptFromString:@"return 4 * 256; "];
    XCTAssertNotNil(value, "value is nil");
    NSLog(@"retValue = %@", value);

    value = [self.context evalScriptFromString:@"return 4 * 256, 'aa', 'bb'; "];
    XCTAssertNotNil(value, "value is nil");
    NSLog(@"retValue = %@", value);
}


/**
 预期结果
 retValue = (
    1024,
    1111,
    Hello
 )
 */
- (void)testEvalScriptFromFile
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    NSString *path = [bundle pathForResource:@"Test" ofType:@"lua"];
    XCTAssertNotNil(path, "path is nil");
    LSCValue *value = [self.context evalScriptFromFile:path];
    XCTAssertNotNil(value, "value is nil");
    NSLog(@"retValue = %@", value);
}


/**
 预期结果：
 resValue = 85
 resValue = (
    30,
    55
 )
 */
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


/**
 预期结果：
 */
- (void)testRegisterModule
{
    LSCValue *resValue = [self.context evalScriptFromString:@"return TestModule:test();"];
    XCTAssertTrue([[resValue toString] isEqualToString:@"Hello World!"], "result value is not equal 'Hello World!'");
    resValue = [self.context evalScriptFromString:@"return TestModule:test('abc');"];
    XCTAssertTrue([[resValue toString] isEqualToString:@"test msg = abc"], "result value is not equal 'Hello World!'");
}


/**
 预期结果：
 [Person object<0x7fa13df686b8>]
 person name = vim
 vim speak:"Hello World!"
 vim 1 years old
 vim pos = 30
 test msg
 */
- (void)testRegisterClass
{
    [self.context evalScriptFromString:@"function Person.prototype:test () print('test msg'); end local p = Person:createPerson(); print(p); p.name = 'vim'; Person:printPersonName(p); p:speak('Hello World!'); p:speak(true); p:speak(30); p:test();"];
}

/**
 预期结果：
 [Chinese object<0x7fbf6a516f48>]
 [Person object<0x7fbf6a517968>]
 */
- (void)testRegClass2
{
    [self.context evalScriptFromString:@"local chinese = Chinese(); print (chinese); local p = Person(); print(p);"];
}


/**
 预期结果：
 xxx
 [Person object<0x7fe9efc0b038>]
 */
- (void)testCreateObjectWithParams
{
    [self.context evalScriptFromString:@"function Person.prototype:init(value) print(value); end local p = Person('xxx'); print(p);"];
}

/**
 预期结果：
 Person
 */
- (void)testClassModuleName
{
    [self.context evalScriptFromString:@"print(Person.name);"];
}


/**
 预期结果：
 true
 false
 */
- (void)testClassIsSubclassOf
{
    [self.context evalScriptFromString:@"print(Person:subclassOf(Object));print(Object:subclassOf(Person));"];
}


/**
 预期结果：
 true
 true
 false
 true
 */
- (void)testClassIsInstanceOf
{
    [self.context evalScriptFromString:@"local p = Person(); print(p:instanceOf(Person)); print(p:instanceOf(Object)); local o = Object(); print(o:instanceOf(Person)); print(o:instanceOf(Object));"];
}


/**
 预期结果：
 [Object type]
 [Person prototype]
 Person init
 Chinese init
 [ChinesePerson object<0x7f9a83147428>]
 [Object prototype]
 Person init
 [Person object<0x7f9a83148238>]
 */
- (void)testClassMethodInherited
{
    [self.context evalScriptFromString:@"print(Object); Person:subclass('ChinesePerson'); function Person.prototype:init () print(self.super); print ('Person init') end function ChinesePerson.prototype:init () ChinesePerson.super.prototype.init(self); print ('Chinese init'); end local c = ChinesePerson(); print(c); local p = Person(); print(p);"];
}


/**
 预期结果：
 1.0
 test = 1
 */
- (void)testSetGlobalVar
{
    [self.context setGlobalWithValue:[LSCValue objectValue:@(1)] forName:@"test"];
    [self.context evalScriptFromString:@"print(test);"];
    
    LSCValue *value = [self.context getGlobalForName:@"test"];
    NSLog(@"test = %@", value);
}


/**
 预期结果：
 1000.0    24.0
 */
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


/**
 预期结果：
 Hello    World
 */
- (void)testModuleTupleReturnValue
{
    [self.context evalScriptFromString:@"local a,b = TestModule:testTuple(); print(a, b);"];
}

/**
 预期结果：
 Hello    World
 */
- (void)testClassInstanceTupleReturnValue
{
    [self.context evalScriptFromString:@"local p = Person(); local a,b = p:test(); print(a, b);"];
}


/**
 预期结果：
 test 1
 retValue = (
    1,
    "Hello World",
    3
 )
 test 2
 retValue = xxxx
 test 3
 retValue =
 */
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


/**
 预期结果：
 [Person type]
 [NativePerson object<0x7f9e0a5a1218>]
 abc speak:"Hello World!"
 */
- (void)testObjProxy
{
    [self.context evalScriptFromString:@"print(Person); local p = Person:createNativePerson(); print(p); p.name = 'abc'; p:speak('Hello World!');"];
}


/**
 预期结果：
 [Person type]    [NativePerson type]
 [Person object<0x7ff08c0ba6e8>]
 abc speak:"Hello World!"
 */
- (void)testClassImportAndObjectClass
{
    [self.context evalScriptFromString:@"print(Person, NativePerson); local p = NativePerson:createPerson(); print(p); p.name = 'abc'; p:speak('Hello World!');"];
}


/**
 预期结果：
 test func
 -------------1
 test func
 -------------2
 */
- (void)testRetainRelease
{
    
    [self.context evalScriptFromString:@"local test = function() print('test func') end; test(); Person:retainHandler(test);"];
    [self.context evalScriptFromString:@"print('-------------1'); Person:callHandler(); Person:releaseHandler();"];
    [self.context evalScriptFromString:@"print('-------------2'); Person:callHandler();"];
}


/**
 预期结果：
 ------    100.0    924.0
 92400
 */
- (void)testRetainRelease_2
{
    [self.context evalScriptFromString:@"function getFunc() return function (a, b) print(\"------\", a, b); return a * b; end end"];
    LSCValue *funcValue = [self.context callMethodWithName:@"getFunc" arguments:nil];
    
    LSCValue *retValue = [[funcValue toFunction] invokeWithArguments:@[[LSCValue numberValue:@100], [LSCValue numberValue:@924]]];
    NSLog(@"%@", retValue);
}

/**
 预期结果：
 test func
 -------------1
 test func
 -------------2
 */
- (void)testRetainRelease_3
{
    [self.context evalScriptFromString:@"local test = function() print('test func') end; test(); Person:retainHandler2(test);"];
    [self.context evalScriptFromString:@"print('-------------1'); Person:callHandler2(); Person:releaseHandler2();"];
    [self.context evalScriptFromString:@"print('-------------2'); Person:callHandler2();"];
}


/**
 预期结果：
 ------ value =     1024.0
 ------ r, g, b =     100.0    20.0    232.0
 [TestModule object<0x7f8843324548>]
 +++++++++++++
 [Person object<0x7f8843326598>]
 vimfung speak:"i am vim"
 [Person object<0x7f884332af08>]
 vim
 person name = vim
 [Person type]    [NativePerson type]
 [Person object<0x7f8841d0e768>]
 abc speak:"Hello World!"
 */
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
    
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    NSString *path = [bundle pathForResource:@"coroutine" ofType:@"lua"];
    [self.context evalScriptFromFile:path];
}


/**
 预期结果：
 [SubLuaLog type]
 ChildLog object init
 [SubLuaLog object<0x7ffdd2e29be8>]
 aaaa
 <SubLuaLog: 0x7ffdd2e29bf0> name = vim
 */
- (void)testNewTypeExporter
{
    [self.context evalScriptFromString:@"Object:typeMapping('ios', 'SubLuaLog', 'ChildLog'); print(ChildLog); function ChildLog.prototype:init () print('ChildLog object init'); end; local t = ChildLog(); print(t); t.xxx = 'aaaa'; print (t.xxx); t.name = 'vim'; t:printName();"];
}


/**
 预期结果
 -------set age    nil
 +++++++set age    12
 *********get age    12
 12.0
 +++++++++ set name
 +++++++++ get name
 vim
 */
- (void)testDefinedProperty
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests_iOS class]];
    NSString *path = [bundle pathForResource:@"definedProperty" ofType:@"lua"];
    [self.context evalScriptFromFile:path];
}


/**
 预期结果：
 [Test type]
 [Test object<0x7fe0ead45108>]
 true
 [TestChild type]
 true
 */
- (void)testSubclass
{
    [self.context evalScriptFromString:@"Object:subclass('Test'); print(Test); local t = Test(); print(t); print(t:instanceOf(Test)); Test:subclass('TestChild'); print(TestChild); local tc = TestChild(); print(tc:instanceOf(Test));"];
}


/**
 预期结果：
 %sadfml
 */
- (void)testGsub
{
    [self.context evalScriptFromString:@"local expertValue=\"%sadfml\"; expertValue = string.gsub(expertValue,\"%%28\",\"%(\"); print(expertValue);"];
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
    self.context = nil;
}

@end
