//
//  LuaScriptCoreTests.m
//  LuaScriptCoreTests
//
//  Created by 冯鸿杰 on 2020/8/11.
//  Copyright © 2020年 vimfung. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "LuaScriptCore.h"
#import "LSCExportTypeModuleHeader.h"
#import <objc/message.h>
#import <objc/runtime.h>

@interface LuaScriptCoreTests : XCTestCase

@property (nonatomic, strong) LSCContext *context;

@end

@implementation LuaScriptCoreTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    self.context = [[LSCContext alloc] init];
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    self.context = nil;
}

- (void)testEvalScript
{
    //执行脚本
    [self.context evalScriptFromString:@"print('Hello World!')"];
    XCTAssertThrows([self.context evalScriptFromString:nil]);
    
    //执行脚本文件
    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSString *filePath = [bundle pathForResource:@"test_file_1" ofType:@"lua"];
    [self.context evalScriptFromFile:filePath];
    XCTAssertThrows([self.context evalScriptFromFile:nil]);
    XCTAssertThrows([self.context evalScriptFromFile:@"notExistsFileName"]);
    
    //nil返回值
    id<LSCValueType> returnValue = [self.context evalScriptFromString:@"return nil"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCNilValue class]]);
    
    //字符串返回值
    returnValue = [self.context evalScriptFromString:@"return 'Hello World!'"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCStringValue class]]);
    
    //数值返回值
    returnValue = [self.context evalScriptFromString:@"return 1"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCNumberValue class]]);
    
    //布尔返回值
    returnValue = [self.context evalScriptFromString:@"return true"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCBooleanValue class]]);
    
    //数组返回值
    returnValue = [self.context evalScriptFromString:@"return {1,2,3,4,5}"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCArrayValue class]]);
    
    //字典返回值
    returnValue = [self.context evalScriptFromString:@"return {k=1, b='Hello', m=true, h={1,2,3}}"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCDictionaryValue class]]);
    
    //元组返回值
    returnValue = [self.context evalScriptFromString:@"return 'Hello', 'World'"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCTupleValue class]]);
    
    //方法返回值
    returnValue = [self.context evalScriptFromString:@"return function () print('Hello World!'); end"];
    XCTAssertTrue([returnValue isKindOfClass:[LSCFunctionValue class]]);
}

- (void)testOnException
{
    [self.context onException:^(LSCContext *context, NSString *message) {
       
        NSLog(@"Exception : %@", message);
        
    }];
    
    //脚本错误
    [self.context evalScriptFromString:@"return function () {print('Hello World');};"];
    [self.context evalScriptFromString:@"error('test exception');"];
    
    //调用方法
    LSCFunctionValue *func = [self.context evalScriptFromString:@"return function () error('An error occurred'); end"];
    [func invokeWithArguments:nil context:self.context];
}

- (void)testDict
{
    LSCDictionaryValue *dictValue = [self.context evalScriptFromString:@"testDict = {a=1, b='Hello', d={e=true}}; return testDict"];
    [dictValue setObject:[LSCValue createValue:@1024] forKeyPath:@"c"];
    [dictValue setObject:[LSCValue createValue:@"World"] forKeyPath:@"a"];
    [dictValue setObject:[LSCValue createValue:@NO] forKeyPath:@"d.f"];
    dictValue = [self.context evalScriptFromString:@"print(testDict.a, testDict.b, testDict.c, testDict.d.e, testDict.d.f); return testDict;"];
    NSDictionary *dict = dictValue.rawValue;
    XCTAssertTrue([dict[@"a"] isEqualToString:@"World"]
                  && [dict[@"b"] isEqualToString:@"Hello"]
                  && [dict[@"c"] isEqual:@1024]
                  && [dict[@"d"][@"e"] isEqual:@YES]
                  && [dict[@"d"][@"f"] isEqual:@NO]);
}

- (void)testFunction
{
    //本地方法 - 不带参数
    LSCFunctionValue *funcValue = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {

        return [LSCValue createValue:@1024];

    }];
    [self.context setGlobal:funcValue forName:@"func"];
    id<LSCValueType> returnValue = [self.context evalScriptFromString:@"return func();"];
    XCTAssertTrue([(NSNumber *)returnValue.rawValue integerValue] == 1024);

    returnValue = [funcValue invokeWithArguments:nil context:self.context];
    XCTAssertTrue([(NSNumber *)returnValue.rawValue integerValue] == 1024);

    //本地方法 - 带参数
    LSCFunctionValue *argFuncValue = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {

        NSInteger v1 = [arguments[0].rawValue integerValue];
        NSInteger v2 = [arguments[1].rawValue integerValue];

        return [LSCValue createValue:@(v1 + v2)];

    }];
    [self.context setGlobal:argFuncValue forName:@"func2"];
    id<LSCValueType> returnValue2 = [self.context evalScriptFromString:@"return func2(1000, 24);"];
    XCTAssertTrue([(NSNumber *)returnValue2.rawValue integerValue] == 1024);

    returnValue2 = [argFuncValue invokeWithArguments:@[[LSCValue createValue:@1000], [LSCValue createValue:@24]]
                                            context:self.context];
    XCTAssertTrue([(NSNumber *)returnValue2.rawValue integerValue] == 1024);

    //lua方法 - 不带参数
    LSCFunctionValue *funcValue3 = [self.context evalScriptFromString:@"return function () return 1024; end"];
    id<LSCValueType> returnValue3 = [funcValue3 invokeWithArguments:nil context:self.context];
    XCTAssertTrue([(NSNumber *)returnValue3.rawValue integerValue] == 1024);
    
    //lua方法 - 带参数
    LSCFunctionValue *funcValue4 = [self.context evalScriptFromString:@"return function (v1, v2) return v1 + v2; end"];
    id<LSCValueType> returnValue4 = [funcValue4 invokeWithArguments:@[[LSCValue createValue:@1000], [LSCValue createValue:@24]] context:self.context];
    XCTAssertTrue([(NSNumber *)returnValue4.rawValue integerValue] == 1024);
    
}

- (void)testGlobal
{
    //nil
    id<LSCValueType> value = [LSCValue createValue:nil];
    [self.context setGlobal:value forName:@"nilValue"];
    [self.context evalScriptFromString:@"print(nilValue)"];
    value = [self.context getGlobalForName:@"nilValue"];
    XCTAssertTrue([value isKindOfClass:[LSCNilValue class]]);
    
    //String
    value = [LSCValue createValue:@"Hello World!"];
    [self.context setGlobal:value forName:@"stringValue"];
    [self.context evalScriptFromString:@"print(stringValue)"];
    value = [self.context getGlobalForName:@"stringValue"];
    XCTAssertTrue([value isKindOfClass:[LSCStringValue class]]);
    
    //Number
    value = [LSCValue createValue:@1024];
    [self.context setGlobal:value forName:@"numberValue"];
    [self.context evalScriptFromString:@"print(numberValue)"];
    value = [self.context getGlobalForName:@"numberValue"];
    XCTAssertTrue([value isKindOfClass:[LSCNumberValue class]]);
    
    //Boolean
    value = [LSCValue createValue:@YES];
    [self.context setGlobal:value forName:@"booleanValue"];
    [self.context evalScriptFromString:@"print(booleanValue)"];
    value = [self.context getGlobalForName:@"booleanValue"];
    XCTAssertTrue([value isKindOfClass:[LSCBooleanValue class]]);
    
    //TupleValue
    LSCTupleValue *tuple = [[LSCTupleValue alloc] init];
    [tuple addObject:@(YES)];
    [tuple addObject:@(1024)];
    [tuple addObject:@"Hello World"];
    [tuple addObject:@[@1,@2,@3]];
    [tuple addObject:@{@"aa" : @1, @"bb" : @YES}];
    value = tuple;
    XCTAssertThrows([self.context setGlobal:value forName:@"tupleValue"]);
    
    //Array
    value = [LSCValue createValue:@[@1, @YES, @"Hello"]];
    [self.context setGlobal:value forName:@"arrayValue"];
    [self.context evalScriptFromString:@"print(arrayValue)"];
    value = [self.context getGlobalForName:@"arrayValue"];
    XCTAssertTrue([value isKindOfClass:[LSCArrayValue class]]);
    
    //Dictionary
    value = [LSCValue createValue:@{@"a" : @1, @"b" : @YES, @"c" : @"Hello"}];
    [self.context setGlobal:value forName:@"dictValue"];
    [self.context evalScriptFromString:@"print(dictValue)"];
    value = [self.context getGlobalForName:@"dictValue"];
    XCTAssertTrue([value isKindOfClass:[LSCDictionaryValue class]]);
    
    //Function
    value = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {
       
        NSLog(@"-------- native method call");
        return nil;
        
    }];
    [self.context setGlobal:value forName:@"func"];
    [self.context evalScriptFromString:@"func()"];
    value = [self.context getGlobalForName:@"func"];
    XCTAssertTrue([value isKindOfClass:[LSCFunctionValue class]]);
    
    //Unknown object
    value = [LSCValue createValue:[NSFileManager defaultManager]];
    [self.context setGlobal:value forName:@"unknownObj"];
    [self.context evalScriptFromString:@"print(unknownObj)"];
    value = [self.context getGlobalForName:@"unknownObj"];
    XCTAssertTrue([value.rawValue isKindOfClass:[NSFileManager class]]);
}

- (void)testCoroutine
{
    XCTestExpectation *expe = [[XCTestExpectation alloc] initWithDescription:@"Coroutine done!"];
    XCTestExpectation *expe2 = [[XCTestExpectation alloc] initWithDescription:@"Coroutine2 done!"];
    XCTestExpectation *expe3 = [[XCTestExpectation alloc] initWithDescription:@"Coroutine3 done!"];
    XCTestExpectation *expe4 = [[XCTestExpectation alloc] initWithDescription:@"Coroutine4 done!"];
    XCTestExpectation *expe5 = [[XCTestExpectation alloc] initWithDescription:@"Coroutine5 done!"];
    XCTestExpectation *expe6 = [[XCTestExpectation alloc] initWithDescription:@"Coroutine6 done!"];
    
    //主线程协程 - 不带参数
    LSCFunctionValue *funcValue = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {

        NSLog(@"Hello World!");

        return nil;

    }];

    LSCCoroutine *coroutine = [[LSCCoroutine alloc] initWithContext:self.context handler:funcValue queue:nil];
    coroutine.resultHandler = ^(LSCCoroutine *coroutine, id<LSCValueType> resultValue) {

        NSLog(@"coroutine result = %@", resultValue);
        [expe fulfill];
    };

    [coroutine resumeWithArguments:nil];

    //主线程协程 - 带参数
    LSCFunctionValue *funcValue2 = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {

        NSInteger v1 = [arguments[0].rawValue integerValue];
        NSInteger v2 = [arguments[1].rawValue integerValue];

        return [LSCValue createValue:@(v1 + v2)];

    }];

    LSCCoroutine *coroutine2 = [[LSCCoroutine alloc] initWithContext:self.context handler:funcValue2 queue:nil];
    coroutine2.resultHandler = ^(LSCCoroutine *coroutine, id<LSCValueType> resultValue) {

        NSLog(@"coroutine2 result = %@", resultValue.rawValue);
        [expe2 fulfill];

    };
    [coroutine2 resumeWithArguments:@[[LSCValue createValue:@1000], [LSCValue createValue:@24]]];
    
    //主线程协程 - 挂起
    LSCFunctionValue *funcValue3 = [self.context evalScriptFromString:@"return function (v1, v2) coroutine.yield(v1 + v2); return 'Hello World!'  end"];
    
    LSCCoroutine *coroutine3 = [[LSCCoroutine alloc] initWithContext:self.context handler:funcValue3 queue:nil];
    coroutine3.resultHandler = ^(LSCCoroutine *coroutine, id<LSCValueType> resultValue) {
        
        NSLog(@"coroutine3 result = %@", resultValue.rawValue);
        if ([resultValue isKindOfClass:[LSCNumberValue class]])
        {
            if ([(NSNumber *)resultValue.rawValue isEqualToNumber:@1024])
            {
                [coroutine resumeWithArguments:@[[LSCValue createValue:@2000], [LSCValue createValue:@48]]];
            }
            
        } else {
            
            [expe3 fulfill];
            
        }
        
    };
    [coroutine3 resumeWithArguments:@[[LSCValue createValue:@1000], [LSCValue createValue:@24]]];
    
    //线程协程 - 无参数
    dispatch_queue_t queue1 = dispatch_queue_create("coroutineQueue", DISPATCH_QUEUE_CONCURRENT);
    LSCFunctionValue *funcValue4 = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {
        
        int index = 0;
        while (index < 5)
        {
            NSLog(@"Other thread:z Hello World!");
            index ++;
            [NSThread sleepForTimeInterval:3];
        }
        
        
        return nil;
        
    }];
    
    LSCCoroutine *coroutine4 = [[LSCCoroutine alloc] initWithContext:self.context handler:funcValue4 queue:queue1];
    coroutine4.resultHandler = ^(LSCCoroutine *coroutine, id<LSCValueType> resultValue) {
        
        NSLog(@"coroutine4 result = %@", resultValue);
        [expe4 fulfill];
    };
    
    [coroutine4 resumeWithArguments:nil];
    
    //线程协程 - 带参数
    dispatch_queue_t queue2 = dispatch_queue_create("coroutineQueue", DISPATCH_QUEUE_CONCURRENT);
    LSCFunctionValue *funcValue5 = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {
        
        NSInteger v1 = [arguments[0].rawValue integerValue];
        NSInteger v2 = [arguments[1].rawValue integerValue];
        
        return [LSCValue createValue:@(v1 + v2)];
        
    }];
    
    LSCCoroutine *coroutine5 = [[LSCCoroutine alloc] initWithContext:self.context handler:funcValue5 queue:queue2];
    coroutine5.resultHandler = ^(LSCCoroutine *coroutine, id<LSCValueType> resultValue) {
        
        NSLog(@"coroutine5 result = %@", resultValue.rawValue);
        [expe5 fulfill];
        
    };
    [coroutine5 resumeWithArguments:@[[LSCValue createValue:@1000], [LSCValue createValue:@24]]];
    
    //线程协程 - 挂起
    dispatch_queue_t queue3 = dispatch_queue_create("coroutineQueue", DISPATCH_QUEUE_CONCURRENT);
    LSCFunctionValue *funcValue6 = [self.context evalScriptFromString:@"return function (v1, v2) coroutine.yield(v1 + v2); return 'Hello World!'  end"];
    
    LSCCoroutine *coroutine6 = [[LSCCoroutine alloc] initWithContext:self.context handler:funcValue6 queue:queue3];
    coroutine6.resultHandler = ^(LSCCoroutine *coroutine, id<LSCValueType> resultValue) {
        
        NSLog(@"coroutine6 result = %@", resultValue.rawValue);
        if ([resultValue isKindOfClass:[LSCNumberValue class]])
        {
            if ([(NSNumber *)resultValue.rawValue isEqualToNumber:@1024])
            {
                [coroutine resumeWithArguments:@[[LSCValue createValue:@2000], [LSCValue createValue:@48]]];
            }
            
        } else {
            
            [expe6 fulfill];
            
        }
        
    };
    [coroutine6 resumeWithArguments:@[[LSCValue createValue:@1000], [LSCValue createValue:@24]]];
    
    [self waitForExpectations:@[expe, expe2, expe3, expe4, expe5, expe6] timeout:1000];
}

- (void)testExportTypeDestroy
{
    XCTestExpectation *expe = [[XCTestExpectation alloc] initWithDescription:@"testExportTypeDestroy done!"];
    
    LSCContext *context = [[LSCContext alloc] init];
    [context useModule:[LSCExportTypeModule class]];
    
    [context evalScriptFromString:@"print(LuaTestType);"];
    
    context = nil;
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            [expe fulfill];
        });
    });
    
    [self waitForExpectations:@[expe] timeout:3];
}

- (void)testInstanceDestroy
{
    XCTestExpectation *expe = [[XCTestExpectation alloc] initWithDescription:@"testInstanceDestroy done!"];
    LSCFunctionValue *compFunc = [[LSCFunctionValue alloc] initWithHandler:^id<LSCValueType>(NSArray<id<LSCValueType>> *arguments) {
       
        [expe fulfill];
        return nil;
        
    }];
    [self.context setGlobal:compFunc forName:@"comp"];
    
    [self.context useModule:[LSCExportTypeModule class]];
    [self.context evalScriptFromString:@"LuaTestType.prototype.destroy = function(self) comp(); end local obj = LuaTestType(); obj=nil;"];
    
    [self waitForExpectations:@[expe] timeout:3];
}

- (void)testExportInstanceMethodWithoutPropertySelector
{
    [self.context useModule:[LSCExportTypeModule class]];
    [self.context evalScriptFromString:@"print(LuaChildType)"];
}

- (void)testExportType
{
    [self.context useModule:[LSCExportTypeModule class]];
    
    NSLog(@"------- begin test export type");
    [self.context evalScriptFromString:@"print(LuaTestType);"];
    [self.context evalScriptFromString:@"print(Object);"];

    [self.context evalScriptFromString:@"print(LuaTestType.super)"];
    [self.context evalScriptFromString:@"print(Object.super);"];

    [self.context evalScriptFromString:@"print(LuaTestType.name)"];
    [self.context evalScriptFromString:@"print(Object.name)"];

    [self.context evalScriptFromString:@"print(LuaTestType.prototype)"];
    [self.context evalScriptFromString:@"print(Object.prototype)"];
    NSLog(@"------- end test export type");

    //subclassOf
    NSLog(@"------- begin test subclass of");
    [self.context evalScriptFromString:@"print(LuaTestType:subclassOf(Object))"];
    [self.context evalScriptFromString:@"print(Object:subclassOf(LuaTestType))"];
    [self.context evalScriptFromString:@"print(LuaTestType.subclassOf(Object))"];
    [self.context evalScriptFromString:@"print(LuaTestType.subclassOf())"];
    NSLog(@"------- end test subclass of");

    //subclass alias
    NSLog(@"------- begin test subclass and alias");
    [self.context evalScriptFromString:@"LuaTestType:subclass('AA'); print(AA);"];
    [self.context evalScriptFromString:@"LuaTestType:alias('TestType'); print(TestType); TestType:alias('LuaTestType')"];
    NSLog(@"------- end test subclass and alias");

    //class method
    NSLog(@"------- begin test call class method");
    id<LSCValueType> retValue1 = [self.context evalScriptFromString:@"return LuaTestType:callMethod();"];
    XCTAssertTrue([retValue1 isKindOfClass:[LSCNilValue class]]);

    id<LSCValueType> retValue2 = [self.context evalScriptFromString:@"return LuaTestType:callMethodIntValue(1024);"];
    XCTAssertTrue([retValue2 isKindOfClass:[LSCNumberValue class]]);

    id<LSCValueType> retValue3 = [self.context evalScriptFromString:@"return LuaTestType:callMethod(nil, 502.12);"];
    XCTAssertTrue([retValue3 isKindOfClass:[LSCNumberValue class]]);

    id<LSCValueType> retValue4 = [self.context evalScriptFromString:@"return LuaTestType:callMethod(nil, true);"];
    XCTAssertTrue([retValue4 isKindOfClass:[LSCBooleanValue class]]);

    id<LSCValueType> retValue5 = [self.context evalScriptFromString:@"return LuaTestType:callMethod(nil, 'Hello World!');"];
    XCTAssertTrue([retValue5 isKindOfClass:[LSCStringValue class]]);
    NSLog(@"------- end test call class method");

    NSLog(@"------- begin test construct instance");
    id<LSCValueType> retValue6 = [self.context evalScriptFromString:@"local obj = LuaTestType(); print(obj); print(obj.class); return obj;"];
    XCTAssertTrue([retValue6.rawValue isKindOfClass:[LSCInstance class]]);
    NSLog(@"------- end test construct instance");

    NSLog(@"------- begin test instance of method");
    id<LSCValueType> retValue7 = [self.context evalScriptFromString:@"local obj = LuaTestType(); return obj:instanceOf(Object), obj:instanceOf(LuaTestType);"];
    XCTAssertTrue([[(LSCTupleValue *)retValue7 objectAtIndex:0] boolValue] == YES
                  && [[(LSCTupleValue *)retValue7 objectAtIndex:1] boolValue] == YES);

    id<LSCValueType> retValue8 = [self.context evalScriptFromString:@"local obj = Object(); return obj:instanceOf(Object), obj:instanceOf(LuaTestType);"];
    XCTAssertTrue([[(LSCTupleValue *)retValue8 objectAtIndex:0] boolValue] == YES
                  && [[(LSCTupleValue *)retValue8 objectAtIndex:1] boolValue] == NO);
    NSLog(@"------- end test instance of method");

    NSLog(@"------- begin test instance property");
    id<LSCValueType> retValue9 = [self.context evalScriptFromString:@"local obj = LuaTestType(); print(obj.A); obj.A = 'test'; print(obj.A); return obj.A"];
    XCTAssertTrue([retValue9.rawValue isEqualToString:@"test"]);
    id<LSCValueType> retValue10 = [self.context evalScriptFromString:@"local obj = LuaChildType(); print(obj.A); return obj.cStr;"];
    XCTAssertTrue([retValue10.rawValue isEqualToString:@"Hello"]);
    id<LSCValueType> retValue11 = [self.context evalScriptFromString:@"local obj = LuaChildType(); print(obj.B); obj.B=1024; return obj.B;"];
    XCTAssertTrue([retValue11.rawValue isEqual:@1024]);
    id<LSCValueType> retValue12 = [self.context evalScriptFromString:@"LuaTestType.prototype.testProp='Hello'; local obj = LuaChildType(); print(obj.testProp); return obj.testProp;"];
    XCTAssertTrue([retValue12.rawValue isEqualToString:@"Hello"]);
    id<LSCValueType> retValue13 = [self.context evalScriptFromString:@"LuaTestType.prototype.name = {get=function (self) return self._name end, set=function(self, val) self._name = val end} local obj = LuaChildType(); obj.name='vimfung'; print(obj.name); return obj.name;"];
    XCTAssertTrue([retValue13.rawValue isEqualToString:@"vimfung"]);
    NSLog(@"------- end test instance property");
    
    NSLog(@"------- begin test dynamic class method");
    id<LSCValueType> retValue14 = [self.context evalScriptFromString:@"LuaTestType.testFunc = function (self) return 1024; end return LuaTestType:testFunc();"];
    XCTAssertTrue([retValue14.rawValue isEqual:@1024]);
    NSLog(@"------- end test dynamic class method");
}

- (void)testStateWatcher
{
    LSCStateWatcher *stateWatcher = [[LSCStateWatcher alloc] initWithEvents:LSCStateWatcherEventLine];
    [stateWatcher onTrigger:^(LSCStateWatcher * _Nonnull watcher, LSCContext * _Nonnull context, LSCMainState * _Nonnull mainState, LSCState * _Nonnull curState) {
       
        NSLog(@"---------- script line");
        
    }];
    
    self.context.stateWatcher = stateWatcher;
    [self.context evalScriptFromString:@"print('Hello World'); return 1024;"];
    
    self.context.stateWatcher = nil;
    [self.context evalScriptFromString:@"print('Hello World no watch');"];
}

- (void)testScriptControlWatcher
{
    XCTestExpectation *exp = [[XCTestExpectation alloc] initWithDescription:@"1"];
    XCTestExpectation *exp2 = [[XCTestExpectation alloc] initWithDescription:@"2"];
    
    NSLog(@"------- begin test main state watcher");
    LSCScriptControlWatcher *w1 = [[LSCScriptControlWatcher alloc] init];
    w1.scriptTimeout = 2;

    self.context.stateWatcher = w1;

    [w1 reset];
    [self.context evalScriptFromString:@"while(1) do print('---------- dosomething'); end"];
    NSLog(@"------- end test main state watcher");
    
    NSLog(@"------- begin test coroutine watcher");
    LSCScriptControlWatcher *w2 = [[LSCScriptControlWatcher alloc] init];
    w2.scriptTimeout = 2;
    [w2 onTimeout:^(LSCScriptControlWatcher * _Nonnull watcher, LSCContext * _Nonnull context) {

        [exp fulfill];

    }];
    LSCFunctionValue *func = [self.context evalScriptFromString:@"return function () while(1) do print('***** coroutine dosomething'); end end"];
    dispatch_queue_t queue = dispatch_queue_create("testqueue", DISPATCH_QUEUE_SERIAL);
    LSCCoroutine *coroutine = [[LSCCoroutine alloc] initWithContext:self.context handler:func queue:queue];
    coroutine.watcher = w2;
    [coroutine resumeWithArguments:nil];
    
    LSCScriptControlWatcher *w3 = [[LSCScriptControlWatcher alloc] init];
    [w3 onExit:^(LSCScriptControlWatcher * _Nonnull watcher, LSCContext * _Nonnull context) {
       
        [exp2 fulfill];
        
    }];
    LSCFunctionValue *func2 = [self.context evalScriptFromString:@"return function () while(1) do print('====== coroutine dosomething no timeout'); end end"];
    dispatch_queue_t queue2 = dispatch_queue_create("testqueue2", DISPATCH_QUEUE_SERIAL);
    LSCCoroutine *coroutine2 = [[LSCCoroutine alloc] initWithContext:self.context handler:func2 queue:queue2];
    coroutine2.watcher = w3;
    [coroutine2 resumeWithArguments:nil];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        
        [w3 exitScript];
        
    });
    
    NSLog(@"------- end test coroutine watcher");
    
    [self waitForExpectations:@[exp, exp2] timeout:1000];
}

- (void)testPerformanceExample {
    
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
