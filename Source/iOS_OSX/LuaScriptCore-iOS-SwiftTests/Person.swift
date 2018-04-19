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

@objc(Person)
class Person: NSObject, LuaExportType, LuaExportTypeAnnotation {
    
    deinit {
        print("deinit");
    }
    
    static func typeName() -> String! {
        return "Person";
    }
    
    @objc class func createPerson() -> Person
    {
        return Person();
    }
    
    @objc var name : String = "";
    
    @objc func speak(_ text : String) -> Void
    {
        print("\(name) speak : \(text)");
    }
    
    @objc class func retainHandler (_ handler : LSCFunction) -> Void
    {
        _func = LuaValue(functionValue: LuaFunction(rawFunction: handler));
        Env.defaultContext.retainValue(value: _func!);
    }
    
    @objc class func releaseHandler ()
    {
        if (_func != nil)
        {
            Env.defaultContext.releaseValue(value: _func!);
            _func = nil;
        }
    }
    
    @objc class func callHandler ()
    {
        if _func != nil
        {
            _ = _func!.functionValue.invoke(arguments: Array<LuaValue>());
        }
    }
    
    @objc class func retainHandler2 (_ handler : LSCFunction) -> Void
    {
        _managedFunc = LuaManagedValue(source: LuaValue(functionValue: LuaFunction(rawFunction: handler)), context: Env.defaultContext);
    }
    
    @objc class func releaseHandler2 ()
    {
        _managedFunc = nil;
    }
    
    @objc class func callHandler2 ()
    {
        if _managedFunc != nil
        {
            _ = _managedFunc?.source.functionValue.invoke(arguments: Array<LuaValue>());
        }
    }
}
