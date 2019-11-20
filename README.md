# 功能&特点

LuaScriptCore旨在能够在多种平台上方便地使用Lua。其提供了与多种平台的功能交互，让开发者无须关心Lua与各个平台之间是实现交互的细节，只需要根据自己的业务需求，使用LuaScriptCore提供的方法，轻松简单地实现各种功能。如：

* **原生代码调用Lua中的方法和变量，控制Lua的业务逻辑**

如，Lua中有如下定义

```lua
url = "https://vimfung.github.io/LuaScriptCore/";

function printUrl(url)

  print (url);

end
```

在原生代码中可以如下面操作Lua变量和方法:

**iOS/OSX**

```objc
//获取变量
LSCValue *urlValue = [context getGlobalForName:@"url"];
NSLog(@"url = %@", [urlValue toString]);

//调用方法
[context callMethodWithName:"printUrl" arguments:@[urlValue]];
```

**Android**

```java
//获取变量
LuaValue urlValue = context.getGlobal("url");
Log.d("LuaScriptCore", String.format("url = %s", urlValue.toString()));

//调用方法
context.callMethod("printUrl", new LuaValue[] {urlValue});
```

**Unity3D**

```csharp
//获取变量
LuaValue urlValue = context.getGlobal ("url");
Debug.LogFormat ("url = {0}", urlValue.toString ());

//调用方法
context.callMethod("printUrl", new List<LuaValue>(new LuaValue[] {urlValue}));
```

* **Lua中调用原生提供的方法，让一些Lua无法处理或者耗时的处理交由原生方法实现**

如，原生代码为Lua定义输出日志方法log：

**iOS/OSX**

```objc
[context registerMethodWithName:@"log" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
       
  NSLog(@"%@", [arguments[0] toString]);
  return nil;
  
}];
```

**Android**

```java
context.registerMethod("log", new LuaMethodHandler() {

  @Override
  public LuaValue onExecute(LuaValue[] arguments) {       
    Log.d("LuaScriptCore", arguments[0].toString());
    return null;
  }
  
});
```

**Unity3D**

```csharp
LuaContext.currentContext.registerMethod("log", (List<LuaValue> arguments) => {

  Debug.Log(arguments[0].toString());
  return null;

});
```

在Lua中则可以调用该方法：

```lua
log("Hello World");
```

* **原生代码定义类型直接映射到Lua中使用，让Lua更方便地实现面向对象的编程**

原生代码有如下类型定义：

**iOS**

```objc
@interface LuaType : NSObject <LSCExportType>

// 定义属性和方法...

@end
```

**Android**

```java
class LuaType implements LuaExportType
{
// 定义属性和方法...
}
```

**Unity3D**

```csharp
class LuaType : LuaExportType 
{
// 定义属性和方法...
}
```

则可以在Lua中进行使用，如：

```lua
local obj = LuaType();
print (obj);
```

# 如何使用

## iOS / OS X 平台

关于iOS／OS X平台下如何使用LuaScriptCore，请参考《[iOS/OS X集成使用文档](https://github.com/vimfung/LuaScriptCore/wiki/iOS&OS-X%E9%9B%86%E6%88%90%E4%BD%BF%E7%94%A8%E6%96%87%E6%A1%A3)》

## Android 平台

关于Android平台下如何使用LuaScriptCore，请参考《[Android集成使用文档](https://github.com/vimfung/LuaScriptCore/wiki/Android%E9%9B%86%E6%88%90%E4%BD%BF%E7%94%A8%E6%96%87%E6%A1%A3)》

## Unity3D

关于Unity3D下如何使用LuaScriptCore，请参考《[Unity3D集成使用文档](https://github.com/vimfung/LuaScriptCore/wiki/Unity3D%E9%9B%86%E6%88%90%E4%BD%BF%E7%94%A8%E6%96%87%E6%A1%A3)》

# 注意

目前源码中不带有任何平台的Release库，在运行Sample时需要从[Relases](https://github.com/vimfung/LuaScriptCore/releases)页签中下载最新版本的库并放入Sample项目后运行。

# API文档

- iOS / OS X [[Objective-C](https://github.com/vimfung/LuaScriptCore/wiki/API%E6%96%87%E6%A1%A3_iOS-OS-X_Objective-C)] [[Swift](https://github.com/vimfung/LuaScriptCore/wiki/API%E6%96%87%E6%A1%A3_iOS-OS-X_Swift)]
- Android [[Java](https://github.com/vimfung/LuaScriptCore/wiki/API%E6%96%87%E6%A1%A3_Android_Java)]
- Unity3D [[C#](https://github.com/vimfung/LuaScriptCore/wiki/API%E6%96%87%E6%A1%A3_Unity3D_CS)]

# 最近更新

## Release 2.4.0 - [下载](https://github.com/vimfung/LuaScriptCore/releases/tag/2.4.0)

更新内容：

1. 新增线程执行功能，可以通过LuaContext的runThread方法将一个lua方法执行在不同的线程中。
2. LuaValue新增setObject方法，允许直接为table对象设置和删除键值对，而不是通过返回值的方法进行调整。
3. 新增LuaContext的脚本执行控制接口，可以通过LuaScriptController来强制中断脚本执行。
4. iOS / OSX 平台下增加初始化上下文时传入配置接口，允许导出类方法名称时使用完整名称。
5. 优化addSearchPath方法，可以加入lua文件以外的文件路径
6. 优化Android和Unity3D下的抛出Lua异常操作
7. 修复抛出异常时导致内存泄漏和程序死锁问题
8. 修复iOS / OSX 平台下使用Swift的@objc导出类无法找到问题
9. 修复Android平台下传递数组中包含导出类型对象时产生JNI栈溢出问题。
10. 修复Android平台下，从原生层传入基础类型数组时无法转换到lua中使用问题。
11. 修复Android平台下LuaValue无法识别传入byte[]类型问题。
12. 修复Android平台下，对象方法传入float、int、long类型参数时无法识别问题。
13. 修复Android平台下，对象方法返回值为float时无法识别问题。
14. 修复Android平台下LuaTuple返回基础类型值不正确问题
15. 修复Android平台下LuaTuple设置List类型为返回值时获取不到列表内容问题
16. 修复Android平台下循环调用方法时导致崩溃问题
17. 修复Android平台下创建类对象是内存泄漏问题
18. 修复Unity3D下LuaValue转换为object时，如果数据为数组或者字典里面的元素没有解包问题。

## [更多更新历史](https://github.com/vimfung/LuaScriptCore/wiki/%E6%9B%B4%E6%96%B0%E5%8E%86%E5%8F%B2)

# 建议&支持

如问题请[与我联系](mailto:luascriptcore@163.com)

![QQ技术讨论群](https://cloud.githubusercontent.com/assets/3739609/22011176/a05d3ca6-dcc8-11e6-8378-6ff68fb0ab9c.png)

# 赞助

打开支付宝扫一扫，给予我支持

![打开支付宝扫一扫](https://user-images.githubusercontent.com/3739609/33522029-5dad4d50-d81d-11e7-848d-7f224f8e737d.jpg)
