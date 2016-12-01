// Generated by Apple Swift version 3.0.1 (swiftlang-800.0.58.6 clang-800.0.42.1)
#pragma clang diagnostic push

#if defined(__has_include) && __has_include(<swift/objc-prologue.h>)
# include <swift/objc-prologue.h>
#endif

#pragma clang diagnostic ignored "-Wauto-import"
#include <objc/NSObject.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(SWIFT_TYPEDEFS)
# define SWIFT_TYPEDEFS 1
# if defined(__has_include) && __has_include(<uchar.h>)
#  include <uchar.h>
# elif !defined(__cplusplus) || __cplusplus < 201103L
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
# endif
typedef float swift_float2  __attribute__((__ext_vector_type__(2)));
typedef float swift_float3  __attribute__((__ext_vector_type__(3)));
typedef float swift_float4  __attribute__((__ext_vector_type__(4)));
typedef double swift_double2  __attribute__((__ext_vector_type__(2)));
typedef double swift_double3  __attribute__((__ext_vector_type__(3)));
typedef double swift_double4  __attribute__((__ext_vector_type__(4)));
typedef int swift_int2  __attribute__((__ext_vector_type__(2)));
typedef int swift_int3  __attribute__((__ext_vector_type__(3)));
typedef int swift_int4  __attribute__((__ext_vector_type__(4)));
typedef unsigned int swift_uint2  __attribute__((__ext_vector_type__(2)));
typedef unsigned int swift_uint3  __attribute__((__ext_vector_type__(3)));
typedef unsigned int swift_uint4  __attribute__((__ext_vector_type__(4)));
#endif

#if !defined(SWIFT_PASTE)
# define SWIFT_PASTE_HELPER(x, y) x##y
# define SWIFT_PASTE(x, y) SWIFT_PASTE_HELPER(x, y)
#endif
#if !defined(SWIFT_METATYPE)
# define SWIFT_METATYPE(X) Class
#endif
#if !defined(SWIFT_CLASS_PROPERTY)
# if __has_feature(objc_class_property)
#  define SWIFT_CLASS_PROPERTY(...) __VA_ARGS__
# else
#  define SWIFT_CLASS_PROPERTY(...)
# endif
#endif

#if defined(__has_attribute) && __has_attribute(objc_runtime_name)
# define SWIFT_RUNTIME_NAME(X) __attribute__((objc_runtime_name(X)))
#else
# define SWIFT_RUNTIME_NAME(X)
#endif
#if defined(__has_attribute) && __has_attribute(swift_name)
# define SWIFT_COMPILE_NAME(X) __attribute__((swift_name(X)))
#else
# define SWIFT_COMPILE_NAME(X)
#endif
#if defined(__has_attribute) && __has_attribute(objc_method_family)
# define SWIFT_METHOD_FAMILY(X) __attribute__((objc_method_family(X)))
#else
# define SWIFT_METHOD_FAMILY(X)
#endif
#if defined(__has_attribute) && __has_attribute(noescape)
# define SWIFT_NOESCAPE __attribute__((noescape))
#else
# define SWIFT_NOESCAPE
#endif
#if !defined(SWIFT_CLASS_EXTRA)
# define SWIFT_CLASS_EXTRA
#endif
#if !defined(SWIFT_PROTOCOL_EXTRA)
# define SWIFT_PROTOCOL_EXTRA
#endif
#if !defined(SWIFT_ENUM_EXTRA)
# define SWIFT_ENUM_EXTRA
#endif
#if !defined(SWIFT_CLASS)
# if defined(__has_attribute) && __has_attribute(objc_subclassing_restricted)
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) __attribute__((objc_subclassing_restricted)) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# else
#  define SWIFT_CLASS(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
#  define SWIFT_CLASS_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_CLASS_EXTRA
# endif
#endif

#if !defined(SWIFT_PROTOCOL)
# define SWIFT_PROTOCOL(SWIFT_NAME) SWIFT_RUNTIME_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
# define SWIFT_PROTOCOL_NAMED(SWIFT_NAME) SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_PROTOCOL_EXTRA
#endif

#if !defined(SWIFT_EXTENSION)
# define SWIFT_EXTENSION(M) SWIFT_PASTE(M##_Swift_, __LINE__)
#endif

#if !defined(OBJC_DESIGNATED_INITIALIZER)
# if defined(__has_attribute) && __has_attribute(objc_designated_initializer)
#  define OBJC_DESIGNATED_INITIALIZER __attribute__((objc_designated_initializer))
# else
#  define OBJC_DESIGNATED_INITIALIZER
# endif
#endif
#if !defined(SWIFT_ENUM)
# define SWIFT_ENUM(_type, _name) enum _name : _type _name; enum SWIFT_ENUM_EXTRA _name : _type
# if defined(__has_feature) && __has_feature(generalized_swift_name)
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME) enum _name : _type _name SWIFT_COMPILE_NAME(SWIFT_NAME); enum SWIFT_COMPILE_NAME(SWIFT_NAME) SWIFT_ENUM_EXTRA _name : _type
# else
#  define SWIFT_ENUM_NAMED(_type, _name, SWIFT_NAME) SWIFT_ENUM(_type, _name)
# endif
#endif
#if !defined(SWIFT_UNAVAILABLE)
# define SWIFT_UNAVAILABLE __attribute__((unavailable))
#endif
#if defined(__has_feature) && __has_feature(modules)
@import ObjectiveC;
@import Foundation;
#endif

#import <LuaScriptCore_iOS_Swift/LuaScriptCore_iOS_Swift.h>

#pragma clang diagnostic ignored "-Wproperty-attribute-mismatch"
#pragma clang diagnostic ignored "-Wduplicate-method-arg"
@class LuaValue;

/**
  Lua上下文对象
*/
SWIFT_CLASS("_TtC23LuaScriptCore_iOS_Swift10LuaContext")
@interface LuaContext : NSObject
/**
  初始化LuaContext
*/
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
/**
  添加搜索路径，对于不在应用主Bundle根目录的lua脚本如果需要require时，则需要指定其搜索路径。
  \param path 路径字符串

*/
- (void)addSearchPathWithPath:(NSString * _Nonnull)path;
/**
  运行异常时触发
  \param handler 事件处理器

*/
- (void)onExceptionWithHandler:(void (^ _Nonnull)(NSString * _Nullable))handler;
/**
  解析脚本
  \param script 脚本字符串


  returns:
  返回值对象
*/
- (LuaValue * _Nullable)evalScriptWithScript:(NSString * _Nonnull)script;
/**
  解析脚本
  \param filePath 文件路径


  returns:
  返回值对象
*/
- (LuaValue * _Nullable)evalScriptWithFilePath:(NSString * _Nonnull)filePath;
/**
  调用方法
  \param methodName 方法名称

  \param arguments 参数列表


  returns:
  返回值对象
*/
- (LuaValue * _Nullable)callMethodWithMethodName:(NSString * _Nonnull)methodName arguments:(NSArray<LuaValue *> * _Nonnull)arguments;
/**
  注册方法
  \param methodName 方法名称

  \param block 方法处理

*/
- (void)registerMethodWithMethodName:(NSString * _Nonnull)methodName block:(LuaValue * _Nonnull (^ _Nonnull)(NSArray<LuaValue *> * _Nonnull))block;
/**
  注册模块
  \param moduleClass 模块类型，必须继承于LSCModule

*/
- (void)registerModuleWithModuleClass:(Class _Nonnull)moduleClass;
/**
  反注册模块
  \param moduleClass 模块类型，必须继承于LSCModule

*/
- (void)unregisterModuleWithModuleClass:(Class _Nonnull)moduleClass;
@end

@class LSCFunction;

/**
  Lua的方法对象类
*/
SWIFT_CLASS("_TtC23LuaScriptCore_iOS_Swift11LuaFunction")
@interface LuaFunction : NSObject
/**
  初始化
*/
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param function OC中的Lua方法对象实例

*/
- (nonnull instancetype)initWithRawFunction:(LSCFunction * _Nonnull)rawFunction OBJC_DESIGNATED_INITIALIZER;
/**
  调用方法
  \param arguments 调用参数


  returns:
  返回值
*/
- (LuaValue * _Nonnull)invokeWithArguments:(NSArray<LuaValue *> * _Nonnull)arguments;
@end

@class LSCPointer;

/**
  Lua的指针对象
*/
SWIFT_CLASS("_TtC23LuaScriptCore_iOS_Swift10LuaPointer")
@interface LuaPointer : NSObject
/**
  初始化
*/
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param pointer 原始指针

*/
- (nonnull instancetype)initWithPointer:(void const * _Nonnull)pointer OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param userdata 原始的Userdata引用

*/
- (nonnull instancetype)initWithUserdata:(LSCUserdataRef _Nonnull)userdata OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param rawPointer 原始的LSCPointer对象

*/
- (nonnull instancetype)initWithRawPointer:(LSCPointer * _Nonnull)rawPointer OBJC_DESIGNATED_INITIALIZER;
@end

@class LSCValue;

/**
  Lua值对象
*/
SWIFT_CLASS("_TtC23LuaScriptCore_iOS_Swift8LuaValue")
@interface LuaValue : NSObject
/**
  初始化
*/
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param intValue 整型值

*/
- (nonnull instancetype)initWithIntValue:(NSInteger)intValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param doubleValue 双精度浮点型

*/
- (nonnull instancetype)initWithDoubleValue:(double)doubleValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param booleanValue 布尔值

*/
- (nonnull instancetype)initWithBooleanValue:(BOOL)booleanValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param stringValue 字符串

*/
- (nonnull instancetype)initWithStringValue:(NSString * _Nonnull)stringValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param arrayValue 数组

*/
- (nonnull instancetype)initWithArrayValue:(NSArray * _Nonnull)arrayValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param dataValue 二进制数据

*/
- (nonnull instancetype)initWithDataValue:(NSData * _Nonnull)dataValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param dictionaryValue 字典

*/
- (nonnull instancetype)initWithDictionaryValue:(NSDictionary * _Nonnull)dictionaryValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param objectValue 对象

*/
- (nonnull instancetype)initWithObjectValue:(id _Nonnull)objectValue OBJC_DESIGNATED_INITIALIZER;
/**
  初始化
  \param rawValue 原始的LSCValue对象

*/
- (nonnull instancetype)initWithRawValue:(LSCValue * _Nonnull)rawValue OBJC_DESIGNATED_INITIALIZER;
/**
  转换为整型值
*/
@property (nonatomic, readonly) NSInteger intValue;
/**
  转换为浮点型值
*/
@property (nonatomic, readonly) double doubleValue;
/**
  转换为布尔值
*/
@property (nonatomic, readonly) BOOL booleanValue;
/**
  转换为字符串值
*/
@property (nonatomic, readonly, copy) NSString * _Nonnull stringValue;
/**
  转换为数组
*/
@property (nonatomic, readonly, copy) NSArray * _Nonnull arrayValue;
/**
  转换为字典
*/
@property (nonatomic, readonly, copy) NSDictionary * _Nonnull dictionaryValue;
/**
  转换为对象
*/
@property (nonatomic, readonly) id _Nonnull objectValue;
/**
  转换为二进制数据
*/
@property (nonatomic, readonly, copy) NSData * _Nonnull dataValue;
/**
  转换为方法对象
*/
@property (nonatomic, readonly, strong) LuaFunction * _Nonnull functionValue;
/**
  转换为指针对象
*/
@property (nonatomic, readonly, strong) LuaPointer * _Nonnull pointerValue;
/**
  获取原始的Value值
*/
@property (nonatomic, readonly, strong) LSCValue * _Nonnull rawValue;
@end

#pragma clang diagnostic pop
