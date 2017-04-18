# 功能&特点

LuaScriptCore旨在能够在多种平台上方便地使用Lua。其提供了与各种平台的功能交互（目前支持iOS、Android以及OS X），让开发者无须关心Lua与各个平台之间是实现交互的细节，只需要根据自己的业务需求，使用LuaScriptCore提供的方法，轻松简单地实现各种功能。其能做到：

* **从平台原生层代码调用Lua中的方法和变量，控制Lua的业务逻辑**
* **从Lua中调用原生层提供的方法，让一些Lua无法处理或者耗时的处理交由原生方法实现**
* **从原生层中直接扩展Lua的功能**
* **从原生层定义的类直接映射到Lua中使用，让Lua更方便地实现面向对象的编程**

# 如何使用

## iOS / OS X 平台

关于iOS／OS X平台下如何使用LuaScriptCore，请参考《[iOS/OS X平台文档](https://github.com/vimfung/LuaScriptCore/wiki/iOS-OS-X%E5%B9%B3%E5%8F%B0%E6%96%87%E6%A1%A3)》

## Android 平台

关于Android平台下如何使用LuaScriptCore，请参考《[Android平台文档](https://github.com/vimfung/LuaScriptCore/wiki/Android-%E5%B9%B3%E5%8F%B0%E6%96%87%E6%A1%A3)》

## Unity3D

关于Unity3D下如何使用LuaScriptCore，请参考《[Unity3D集成使用文档](https://github.com/vimfung/LuaScriptCore/wiki/Unity3D%E9%9B%86%E6%88%90%E4%BD%BF%E7%94%A8%E6%96%87%E6%A1%A3)》

# 注意

目前源码中不带有任何平台的Release库，在运行Sample时需要从[Relases](https://github.com/vimfung/LuaScriptCore/releases)页签中下载最新版本的库并放入Sample项目后运行。

# 建议&支持

如有问题请[与我联系](mailto:vimfung@qq.com)

![QQ技术讨论群](https://cloud.githubusercontent.com/assets/3739609/22011176/a05d3ca6-dcc8-11e6-8378-6ff68fb0ab9c.png)
