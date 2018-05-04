# 功能&特点

LuaScriptCore旨在能够在多种平台上方便地使用Lua。其提供了与多种平台的功能交互，让开发者无须关心Lua与各个平台之间是实现交互的细节，只需要根据自己的业务需求，使用LuaScriptCore提供的方法，轻松简单地实现各种功能。如：

* **原生代码调用Lua中的方法和变量，控制Lua的业务逻辑**

如，Lua中有如下定义

```
url = "https://vimfung.github.io/LuaScriptCore/";

function printUrl(url)
{
  print (url);
}
```

在原生代码中可以如下面操作Lua变量和方法:

**iOS/OSX**

```
//获取变量
LSCValue *urlValue = [context getGlobalForName:@"url"];
NSLog(@"url = %@", [urlValue toString]);

//调用方法
[context callMethodWithName:"printUrl" arguments:@[urlValue]];
```

**Android**

```
//获取变量
LuaValue urlValue = context.getGlobal("url");
Log.d("LuaScriptCore", String.format("url = %s", urlValue.toString()));

//调用方法
context.callMethod("printUrl", new LuaValue[] {urlValue});
```

**Unity3D**

```
//获取变量
LuaValue urlValue = context.getGlobal ("url");
Debug.LogFormat ("url = {0}", urlValue.toString ());

//调用方法
context.callMethod("printUrl", new List<LuaValue>(new LuaValue[] {urlValue}));
```

* **Lua中调用原生提供的方法，让一些Lua无法处理或者耗时的处理交由原生方法实现**

如，原生代码为Lua定义输出日志方法log：

**iOS/OSX**

```
[context registerMethodWithName:@"log" block:^LSCValue *(NSArray<LSCValue *> *arguments) {
       
  NSLog(@"%@", [arguments[0] toString]);
  return nil;
  
}];
```

**Android**

```
context.registerMethod("log", new LuaMethodHandler() {

  @Override
  public LuaValue onExecute(LuaValue[] arguments) {       
    Log.d("LuaScriptCore", arguments[0].toString());
    return null;
  }
  
});
```

**Unity3D**

```
LuaContext.currentContext.registerMethod("log", (List<LuaValue> arguments) => {

  Debug.Log(arguments[0].toString());
  return null;

});
```

在Lua中则可以调用该方法：

```
log("Hello World");
```

* **原生代码定义类型直接映射到Lua中使用，让Lua更方便地实现面向对象的编程**

原生代码有如下类型定义：

**iOS**

```
@interface LuaType : NSObject <LSCExportType>

// 定义属性和方法...

@end
```

**Android**

```
class LuaType implements LuaExportType
{
// 定义属性和方法...
}
```

**Unity3D**

```
class LuaType : LuaExportType 
{
// 定义属性和方法...
}
```

则可以在Lua中进行使用，如：

```
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

# 最近更新

## Release 2.2.0 - [下载](https://github.com/vimfung/LuaScriptCore/releases/tag/2.2.0)

本次更新内容包括：

1. 废弃原有构造函数`create`，使用`类型()`方式构造对象；

2. 统一使用冒号来进行类方法和实例方法的声明和调用（之前版本类方法用点语法，实例方法用冒号语法）。

3. 优化类型加载逻辑，提升执行效率。

4. `Object`类型新增`typeMapping`方法，用于为原生类型定义别名。

5. `LuaContext`新增`raiseException`原生方法，用于抛出Lua执行异常。

6. 修复iOS、OSX下Function类型传入原生层后释放时崩溃问题。

7. 修复iOS、OSX下32位设备返回布尔类有误问题。

8. 修复Windows下编译Android项目报错问题。

**注：由于本次更新涉及语法变更，更新后需要调整变更的语法，请使用旧版本的同学谨慎升级**

## [更多更新历史](https://github.com/vimfung/LuaScriptCore/wiki/%E6%9B%B4%E6%96%B0%E5%8E%86%E5%8F%B2)

# 建议&支持

如问题请[与我联系](mailto:vimfung@qq.com)

![QQ技术讨论群](https://cloud.githubusercontent.com/assets/3739609/22011176/a05d3ca6-dcc8-11e6-8378-6ff68fb0ab9c.png)

# 赞助

打开支付宝扫一扫，给予我支持

![打开支付宝扫一扫](https://user-images.githubusercontent.com/3739609/33522029-5dad4d50-d81d-11e7-848d-7f224f8e737d.jpg)
