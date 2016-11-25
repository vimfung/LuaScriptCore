//
//  LuaScriptCoreTests.m
//  LuaScriptCoreTests
//
//  Created by 冯鸿杰 on 16/11/15.
//  Copyright © 2016年 冯鸿杰. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "LuaScriptCoreForUnity.h"
#import "LuaValue.h"
#import "LuaObjectEncoder.hpp"
#import "LuaObjectDecoder.hpp"

@interface LuaScriptCoreTests : XCTestCase

@end

@implementation LuaScriptCoreTests

static void* testHandler(const char *methodName, const void *params, int size)
{
    using namespace cn::vimfung::luascriptcore;
    
    LuaValue *value = LuaValue::IntegerValue(0);
    
    LuaObjectEncoder *encoder = new LuaObjectEncoder();
    encoder -> writeObject(value);
    
    void *retBuf = (void *)encoder -> cloneBuffer();
    
    encoder -> release();
    
    value -> release();
    
    return retBuf;
}

- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testExample
{
    using namespace cn::vimfung::luascriptcore;
    
    int navId = createLuaContext();
//
//    void *result = NULL;
////    evalScript(navId, "print('Hello World!');", (const void **)&result);
//    
//    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests class]];
//    NSString *path = [bundle pathForResource:@"main" ofType:@"lua"];
//    evalScriptFromFile(navId, path.UTF8String, (const void **)&result);
//    
//    free(result);
//    
//    LuaValue *v1 = LuaValue::IntegerValue(12);
//    LuaValue *v2 = LuaValue::IntegerValue(44);
//    
//    LuaObjectEncoder *encoder = new LuaObjectEncoder();
//    encoder -> writeInt32(2);
//    v1 -> serialization("LuaValue", encoder);
//    v2 -> serialization("LuaValue", encoder);
//    
//    callMethod(navId, "add", encoder -> getBuffer(), (const void **)&result);
//    
//    v1 -> release();
//    v2 -> release();
//    encoder -> release();
//    
//    free(result);
    
    LuaValue *value = LuaValue::IntegerValue(-24);
    LuaObjectEncoder *encoder = new LuaObjectEncoder();
    value -> serialization("LuaValue", encoder);
    
    LuaObjectDecoder *decoder = new LuaObjectDecoder(encoder -> getBuffer());
    decoder -> readObject();

    
//    static bool isReg = false;
//    if (!isReg)
//    {
//        registerMethod(navId, "test", testHandler);
//        isReg = true;
//    }
//    
//    void *retBytes = NULL;
//    evalScript(navId, "print(test(1024, 2));", (const void **)&retBytes);
//    free(retBytes);
//    
//    evalScript(navId, "print(test(1024, 2));", (const void **)&retBytes);
//    free(retBytes);
}


@end
