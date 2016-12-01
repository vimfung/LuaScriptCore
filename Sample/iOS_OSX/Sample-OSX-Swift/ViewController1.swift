//
//  ViewController.swift
//  Sample-OSX-Swift
//
//  Created by 冯鸿杰 on 16/12/1.
//  Copyright © 2016年 vimfung. All rights reserved.
//

import Cocoa
import LuaScriptCore_OSX_Swift

class ViewController1: NSViewController {

    var _context : LuaContext = LuaContext();
    var _hasRegMethod : Bool = false;
    var _hasRegModule : Bool = false;
    var _hasRegClass : Bool = false;
    
    override func viewDidLoad() {
        super.viewDidLoad()
    }

    /// 解析脚本按钮点击
    ///
    /// - Parameter sender: 事件对象
    @IBAction func evalScriptButtonClickedHandler(_ sender: Any)
    {
        //解析并执行Lua脚本
        let retValue : LuaValue? = _context.evalScript(script: "print(10);return 'Hello World';");
        NSLog("%@", retValue!.stringValue);
    }
    
    /// 注册方法按钮点击
    ///
    /// - Parameter sender: 事件对象
    @IBAction func regMethodButtonClickedHandler(_ sender: Any)
    {
        if (!_hasRegMethod)
        {
            _hasRegMethod = true;
            
            _context.registerMethod(methodName: "getDeviceInfo", block: { (arguments : Array<LuaValue>) -> LuaValue in
                
                var info : Dictionary<String, String> = Dictionary<String, String>();
                info["key1"] = "value1";
                info["key2"] = "value2";
                info["key3"] = "value3";
                info["key4"] = "value4";
                
                return LuaValue(dictionaryValue: info);
                
            });
        }
        
        //调用脚本
        _ = _context.evalScript(filePath: "main.lua");
    }
    
    /// 调用Lua方法按钮点击
    ///
    /// - Parameter sender: 事件对象
    @IBAction func callLuaMethodButtonClickedHandler(_ sender: Any)
    {
        
        //加载Lua脚本
        _ = _context.evalScript(filePath: "todo.lua");
        
        //调用Lua方法
        let retValue : LuaValue? = _context.callMethod(methodName: "add", arguments: [LuaValue(intValue: 1000), LuaValue(intValue:24)]);
        if (retValue != nil)
        {
            NSLog("result = %d", retValue!.intValue);
        }
    }
    
    /// 注册模块按钮点击
    ///
    /// - Parameter sender: 事件对象
    @IBAction func regModuleButtonClickedHandler(_ sender: Any)
    {
        if (!_hasRegModule)
        {
            _hasRegModule = true;
            _context.registerModule(moduleClass: LogModule.self);
        }
        
        _ = _context.evalScript(script: "LogModule.writeLog('Hello Lua Module!');");
    }
    
    /// 注册类按钮点击
    ///
    /// - Parameter sender: 事件对象
    @IBAction func regClassButtonClickedHandler(_ sender: Any)
    {
        if (!_hasRegClass)
        {
            _hasRegClass = true;
            _context.registerModule(moduleClass: LSCTPerson.self);
        }
        
        _ = _context.evalScript(filePath: "test.lua");
    }
}

