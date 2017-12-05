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
local obj = LuaType.create();
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

## Release 2.0.0 - [下载](https://github.com/vimfung/LuaScriptCore/releases/tag/2.0.0)

LuaScriptCore迎来一次重大更新！本次更新主要涉及对象映射部分功能，其中包括：

- 导出类型更加自由，取消强制继承`Module`或者`ObjectClass`类型的导出机制，只要类型实现`ExportType`则可以在Lua脚本中进行使用。在原生代码定义如下：

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
local obj = LuaType.create();
print (obj);
```

- 导出类型的属性访问更加简单易用，如果的原生类有一个name的属性定义如下：

**iOS**

```
@interface LuaType : NSObject <LSCExportType>

@property (nonatomic, copy) NSString *name;

@end
```

**Android**

```
class LuaType implements LuaExportType
{
    public String name;
}
```

**Unity3D**

```
class LuaType : LuaExportType 
{
    private string _name;
    public string name
    {
        get 
        {
            return _name;
        }
        set
        {
            _name = value;
        }
    }
}
```

以前的版本在Lua中的调用形式如下：

```
local obj = LuaType.create();
obj:setName("vim");
print (obj:name());
```

新版本中调用形式如下：

```
local obj = LuaType.create();
obj.name = "vim";
print (obj.name);
```

- 现在可以支持类型方法的重载了！！假设你的原生类型方法有多个重载：

**iOS**

```
@interface LuaType : NSObject <LSCExportType>

- (void)test;

- (void)testWithMsg:(NSString *)msg;

@end
```

**Android**

```
class LuaType implements LuaExportType
{
    public void test () {};

    public void test(String msg) {};
}
```

**Unity3D**

```
class LuaType : LuaExportType 
{
    public void test () {};

    public void test (string msg) {};
}
```

在Lua中可以根据传参来调用到不同方法重载，如：

```
local obj = LuaType.create();
obj.test();
obj.test("Hello World");
```

- 本次更新为了满足部分同学的需要，**对Lua 5.1版本核心进行了支持**（适用于iOS、OSX、Android平台），但是没有直接制作relase包，有需要的同学可以直接使用源码工程打包。具体打包步骤如下：

**iOS**

1. 打开`Source/iOS_OSX/LuaScriptCore_5_1.xcodeproj`工程
2. 选择`LuaScriptCore-iOS-output`或者`LuaScriptCore-iOS-Swift-output`scheme编译输出iOS包
3. 在`Release/iOS`或者`Release/iOS-Swift`中可找到导出包

**OSX**

1. 打开`Source/iOS_OSX/LuaScriptCore_5_1.xcodeproj`工程
2. 选择`LuaScriptCore-OSX-output`或者`LuaScriptCore-OSX-Swift-output`shcme编译输出OSX包
3. 在`Release/OSX`或者`Release/OSX-Swift`中可找到导出包

**Android**

1. 使用Android Studio 打开`Source/Android`工程
2. 打开`build.gradle(Module:luascriptcore)`文件，将`apply from: './compile.gradle'`改为`apply from: './compile-5.1.5.fradle'`
3. 然后重新编译
4. 在`Source/Android/luascriptcore/build/intermediates/bundles/release/`目录下找到导出包

- 除了上述更新内容外，本次还优化了内部的一些处理逻辑和流程，同时修复了一些已的bug。具体内容包括：

1. 优化iOS／OSX下addSearchPath接口逻辑处理
2. 修复iOS／OSX平台在Xcode9下打包出错问题
3. 修复Context销毁时出现崩溃的问题。
4. 修复在Lua协程操作时访问异常问题。

## [更多更新历史](https://github.com/vimfung/LuaScriptCore/wiki/%E6%9B%B4%E6%96%B0%E5%8E%86%E5%8F%B2)

# 建议&支持

如问题请[与我联系](mailto:vimfung@qq.com)

![QQ技术讨论群](https://cloud.githubusercontent.com/assets/3739609/22011176/a05d3ca6-dcc8-11e6-8378-6ff68fb0ab9c.png)

# 赞助

打开支付宝扫一扫，给予我支持

![打开支付宝扫一扫](https://user-images.githubusercontent.com/3739609/33522029-5dad4d50-d81d-11e7-848d-7f224f8e737d.jpg)
