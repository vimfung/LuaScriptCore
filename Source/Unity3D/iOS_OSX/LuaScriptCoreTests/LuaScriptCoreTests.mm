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
#import "LuaTuple.h"
#import "StringUtils.h"
#import "LuaExportsTypeManager.hpp"

using namespace cn::vimfung::luascriptcore;

@interface LuaScriptCoreTests : XCTestCase

@property (nonatomic) int contextId;

@end

@implementation LuaScriptCoreTests

static void* testHandler(const char *methodName, const void *params, int size)
{
//    using namespace cn::vimfung::luascriptcore;
//    
//    LuaValue *value = LuaValue::IntegerValue(0);
//    
//    LuaObjectEncoder *encoder = new LuaObjectEncoder();
//    encoder -> writeObject(value);
//    
//    void *retBuf = (void *)encoder -> cloneBuffer();
//    
//    encoder -> release();
//    
//    value -> release();
//    
//    return retBuf;
    return NULL;
}

void* testModuleMethodHandler (int moduleId, const char *name, const void *args, int size)
{
    return NULL;
}

- (void)setUp
{
    [super setUp];
    
    self.contextId = createLuaContext();
}

- (void)tearDown
{
    releaseObject(self.contextId);
    [super tearDown];
}

- (void)testExample
{
    using namespace cn::vimfung::luascriptcore;

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
    
//    LuaValue *value = LuaValue::IntegerValue(-24);
//    LuaObjectEncoder *encoder = new LuaObjectEncoder();
//    value -> serialization("LuaValue", encoder);
//    
//    LuaObjectDecoder *decoder = new LuaObjectDecoder(encoder -> getBuffer());
//    decoder -> readObject();

    
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



- (void)testRegModule
{
//    int navId = createLuaContext();
//    
//    cn::vimfung::luascriptcore::LuaObjectEncoder *encoder = new cn::vimfung::luascriptcore::LuaObjectEncoder();
//    encoder -> writeInt32(1);
//    encoder -> writeString("test");
//    
//    registerModule(navId, "LogModule", encoder -> getBuffer(), testModuleMethodHandler);
//    evalScript(navId, "LogModule.test({1,2,3,4,5});", NULL);
}

- (void)testRegClass
{

}

- (void)testTypeIdFeature
{
    LuaValue *luaValue = new LuaValue();
    LuaObject *obj = luaValue;
    NSLog(@"%s", luaValue -> typeName().c_str());
    NSLog(@"%s", obj -> typeName().c_str());
}

- (void)testTuple
{
    NSBundle *bundle = [NSBundle bundleForClass:[LuaScriptCoreTests class]];
    NSString *path = [bundle pathForResource:@"main" ofType:@"lua"];
    evalScriptFromFile(self.contextId, path.UTF8String, NULL);
    
    void *result = NULL;
    int size = callMethod(self.contextId, "testTuple", NULL, (const void **)&result);
    char chr = *((char *)result+87);
    NSLog(@"size = %d, buf[88] = %d", size, chr);
    
    free(result);
}

- (void)testTuple2
{
    LuaContext *context = (LuaContext *)LuaObject::findObject(self.contextId);
    
    Byte bytes[82] = {76,0,0,0,8,76,117,97,86,97,108,117,101,59,0,0,0,0,0,11,76,0,0,0,8,75,117,97,84,117,112,108,101,59,0,0,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,5,72,101,108,108,111,0,0,0,0,0,8,0,0,7,225,0,0,0,0,0,3,0,0,0,5,87,111,114,108,100};
    
    
    
//    LuaObjectEncoder::setMappingClassType(typeid(LuaValue).name(), "LuaValue");
//    LuaObjectEncoder::setMappingClassType(typeid(LuaTuple).name(), "LuaTuple");
//    
//    LuaTuple *tuple = new LuaTuple();
//    tuple -> addReturnValue(LuaValue::StringValue("Hello"));
//    tuple -> addReturnValue(LuaValue::NumberValue(2017));
//    tuple -> addReturnValue(LuaValue::StringValue("World"));
//    
//    LuaValue *value = LuaValue::TupleValue(tuple);
//    
//    LuaObjectEncoder *encoder = new LuaObjectEncoder(context);
//    encoder -> writeObject(value);
//    
    LuaObjectDecoder *decoder = new LuaObjectDecoder(context, bytes);
    LuaValue *value = dynamic_cast<LuaValue *>(decoder -> readObject());
    decoder -> release();
    
    NSLog(@"------- len = %d", value -> getType());
}

- (void)testReturnValue
{
    LuaContext *context = (LuaContext *)LuaObject::findObject(self.contextId);
    
    Byte bytes[77] = {76,0,0,0,8,76,117,97,86,97,108,117,101,59,0,0,0,0,0,7,76,0,0,0,27,76,117,97,79,98,106,101,99,116,73,110,115,116,97,110,99,101,68,101,115,99,114,105,112,116,111,114,59,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,6,80,101,114,115};
    
    LuaObjectDecoder *decoder = new LuaObjectDecoder(context, bytes);
    LuaValue *value = dynamic_cast<LuaValue *>(decoder -> readObject());
    decoder -> release();
    
    NSLog(@"------- len = %d", value -> getType());
    
    value -> push(context);
    value -> release();
}

- (void)testStringFormat
{
    XCTestExpectation *ex = [self expectationWithDescription:@"xxxx"];
    
    LuaContext *context = (LuaContext *)LuaObject::findObject(self.contextId);
    LuaValue *value = context -> evalScript("return function () print('hello world'); end;");
    printf("%p\n", value);
    
    [self waitForExpectations:@[ex] timeout:3600];
}

- (void)testContext
{
    XCTestExpectation *ex = [self expectationWithDescription:@"xxxx"];
    
    LuaContext *context = new LuaContext();
    std::string clsName = "Test";
    LuaExportTypeDescriptor *testType = new LuaExportTypeDescriptor(clsName, NULL);
    context -> getExportsTypeManager() -> exportsType(testType);
    testType -> release();
    context -> evalScript("print(Test);local t = Test.create();");
    context -> release();
    
    [self waitForExpectations:@[ex] timeout:3600];
}

@end
