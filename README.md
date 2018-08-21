# 功能&特点

LuaScriptCore旨在能够在多种平台上方便地使用Lua。其提供了与多种平台的功能交互，让开发者无须关心Lua与各个平台之间是实现交互的细节，只需要根据自己的业务需求，使用LuaScriptCore提供的方法，轻松简单地实现各种功能。如：

* **原生代码调用Lua中的方法和变量，控制Lua的业务逻辑**

如，Lua中有如下定义

```
url = "https://vimfung.github.io/LuaScriptCore/";

function printUrl(url)

  print (url);

end
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

## Release 2.3.0 - [下载](https://github.com/vimfung/LuaScriptCore/releases/tag/2.3.0)

更新内容：

1. 增加线程安全机制
2. lua核心升级至5.3.5版本
3. 修复Android平台下使用`ArrayList`作为参数传入`callMethod`中无效问题。
4. 允许Android、Unity平台下使用泛型作为参数传入lua。
5. 修复Android平台下map中设置null元素转换失败问题
6. Android和Unity平台下的`LuaExportTypeConfig`和`LuaExportTypeAnnotation`标注过期（后续版本将移除），使用`LuaExclude`注解（特性）来代替:

**Android平台**

```
//调整前
@LuaExportTypeConfig(excludeExportInstanceMethodsNames = {"method1", "method2"})
class TargetClass implements LuaExportType
{
    public void method1 () {}
    public void method2 () {}
}

//调整后
class TargetClass implements LuaExportType
{
    @LuaExclude
    public void method1 () {}

    @LuaExclude
    public void method2 () {}
}
```

**Unity平台**

```
//调整前
[LuaExportTypeAnnotation(excludeExportInstanceMethodNames=new string[]{"method1", "method2"})]
class TargetClass : LuaExportType 
{
    public void method1 () {}
    public void method2 () {}
}

//调整后
class TargetClass : LuaExportType 
{
    [LuaExclude]
    public void method1 () {}

    [LuaExclude]
    public void method2 () {}
}
```

## [更多更新历史](https://github.com/vimfung/LuaScriptCore/wiki/%E6%9B%B4%E6%96%B0%E5%8E%86%E5%8F%B2)

# 建议&支持

如问题请[与我联系](mailto:vimfung@qq.com)

![QQ技术讨论群](https://cloud.githubusercontent.com/assets/3739609/22011176/a05d3ca6-dcc8-11e6-8378-6ff68fb0ab9c.png)

# 赞助

打开支付宝扫一扫，给予我支持

![打开支付宝扫一扫](https://user-images.githubusercontent.com/3739609/33522029-5dad4d50-d81d-11e7-848d-7f224f8e737d.jpg)
