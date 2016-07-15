# LuaScriptCore

Use it can be easily applied the Lua script on iOS, and provides a powerful interactive features to be extended

## How to use

### Integrated

1. Clone the source to local disk.
2. Drag the LuaScriptCore.xcodeproj to your project.
3. Click your project root node in Xcode, and go to the Build Phases panel in right panel; 
4. Add LuaScriptCore as dependency library in Target Dependencies section.
5. Add LuaScriptCore.a in Link Binary With Libraries section.
6. Now,you can import the LuaScriptCore/LuaScriptCore.h header in your code then use it.

### Initialze

```
LSCContext *context = [[LSCContext alloc] init];
```

### Eval script

```
LSCValue *value = [self.context callMethodWithName:@"add"
                                         arguments:@[[LSCValue integerValue:1000],
                                                     [LSCValue integerValue:24]]];
NSLog(@"result = %@", [value toNumber]);
```

### Register ObjC method to lua

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

### Calls a lua method

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
