//
//  Person.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/3/22.
//  Copyright © 2017年 vimfung. All rights reserved.
//

import UIKit
import LuaScriptCore_iOS_Swift

private var _func : LuaValue? = nil;
private var _managedFunc : LuaManagedValue? = nil;

class Person: LSCObjectClass {
    
    var _name : String = "";
    
    class func createPerson() -> Person
    {
        return Person();
    }
    
    func setName(name : String) -> Void
    {
        _name = name;
    }
    
    func name() -> String
    {
        return _name;
    }
    
    func speak(text : String) -> Void
    {
        NSLog("%@ speak : %@", _name, text);
    }
    
    class func retainHandler (handler : LSCFunction) -> Void
    {
        _func = LuaValue(functionValue: LuaFunction(rawFunction: handler));
        Env.defaultContext.retainValue(value: _func!);
    }
    
    class func releaseHandler ()
    {
        if (_func != nil)
        {
            Env.defaultContext.releaseValue(value: _func!);
            _func = nil;
        }
    }
    
    class func callHandler ()
    {
        if _func != nil
        {
            _ = _func!.functionValue.invoke(arguments: Array<LuaValue>());
        }
    }
    
    class func retainHandler2 (handler : LSCFunction) -> Void
    {
        _managedFunc = LuaManagedValue(source: LuaValue(functionValue: LuaFunction(rawFunction: handler)), context: Env.defaultContext);
    }
    
    class func releaseHandler2 ()
    {
        _managedFunc = nil;
    }
    
    class func callHandler2 ()
    {
        if _managedFunc != nil
        {
            _ = _managedFunc?.source.functionValue.invoke(arguments: Array<LuaValue>());
        }
    }
}
