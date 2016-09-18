# LuaScriptCore

Use it can be easily applied the Lua script on iOS, Android, OS X, and provides a powerful interactive features to be extended

## iOS / OS X Platform

**Integrated**

1. Clone the source to local disk.
2. Navigate to the “Release” directory, Then select the platform(iOS or OS X) directory.
3. Drag platform directory to your project and check the "Copy items if needed".
4. Now, you can import the LuaScriptCore/LuaScriptCore.h header in your code then use it.

**Rebuild the project**

1. Navigate to the "Source/iOS_OSX/" directory and Open the LuaScriptCore project.
2. Choose what you want to re-build the scheme(Chose the LuaScriptCore-iOS-output scheme is re-build iOS platform, and Chose the LuaScriptCore-OSX-output scheme is re-build OS X platform).
3. Command+B to re-build the project, After build is output to the “Release/platform(iOS/OSX)" directory.

**Initialze**

```
LSCContext *context = [[LSCContext alloc] init];
```

**Eval script**

```
[self.context evalScriptFromString:@"print('Hello World');"];
```

**Register ObjC method to lua**

In ObjC

```
[self.context registerMethodWithName:@"getDeviceInfo" block:^LSCValue *(NSArray *arguments) {
            
    NSMutableDictionary *info = [NSMutableDictionary dictionary];
    [info setObject:[UIDevice currentDevice].name forKey:@"deviceName"];
    [info setObject:[UIDevice currentDevice].model forKey:@"deviceModel"];
    [info setObject:[UIDevice currentDevice].systemName forKey:@"systemName"];
    [info setObject:[UIDevice currentDevice].systemVersion forKey:@"systemVersion"];
    
    return [LSCValue dictionaryValue:info];
    
}];
```

In lua

```
local tbl = getDeviceInfo();
```

**Calls a lua method**

In lua

```
function add (a, b)

    return a+b;

end
```

In ObjC

```
LSCValue *value = [self.context callMethodWithName:@"add"
                                         arguments:@[[LSCValue integerValue:1000],
                                                     [LSCValue integerValue:24]]];
NSLog(@"result = %@", [value toNumber]);
```

## Android Platform

**Integrated**

1. Clone the source to local disk.
2. Navigate to the “Release” directory, Then copy all the contents of the "Android" directory to the Libs folder of the project.
3. Open the build.gradle file of the app module，and set the jniLibs.srcDirs property like this:


    ```

    android {

        //...

        sourceSets {

            main {
                jniLibs.srcDirs "jniLibs"
            }

        }
    }

    ```


4. Now, you can import the LuaContext class in your code then use it.

**Rebuild the project**

1. Open the Android Studio and import the project from the "Source/Android/" directory.
2. Command + F9 rebuild the project.
3. Compile completed that in the module's build/intermediates/bundles/release directory to find the classes.jar and JNI sub directory.

**Initialze**

```
LuaContext context = LuaContext.create(this);
```

**Eval script**

```
context.evalScript("print('Hello World');");
```

**Register ObjC method to lua**

In Android

```
context.registerMethod("getDeviceInfo", new LuaMethodHandler() {
    @Override
    public LuaValue onExecute(LuaValue[] arguments) {

        HashMap devInfoMap = new HashMap();
        devInfoMap.put("deviceName", Build.DISPLAY);
        devInfoMap.put("deviceModel", Build.MODEL);
        devInfoMap.put("systemName", Build.PRODUCT);
        devInfoMap.put("systemVersion", Build.VERSION.RELEASE);

        return new LuaValue(devInfoMap);
    }
});
```

In lua

```
local tbl = getDeviceInfo();
```

**Calls a lua method**

In lua

```
function add (a, b)

    return a+b;

end
```

In Android

```
LuaValue retValue = context.callMethod("add", new LuaValue[]{new LuaValue(100), new LuaValue(924)});
Log.v("luaScriptCore", String.format("%d", retValue.toInteger()));
```


