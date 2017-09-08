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

class Person: NSObject, LuaExportType {
    
    deinit {
        print("deinit");
    }
    
    class func createPerson() -> Person
    {
        return Person();
    }
    
    var name : String = "";
    
    func speak(_ text : String) -> Void
    {
        print("\(name) speak : \(text)");
    }
    
    class func retainHandler (_ handler : LSCFunction) -> Void
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
    
    class func retainHandler2 (_ handler : LSCFunction) -> Void
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
