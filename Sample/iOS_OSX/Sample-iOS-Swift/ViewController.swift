//
//  ViewController.swift
//  Sample-iOS-Swift
//
//  Created by 冯鸿杰 on 16/12/1.
//  Copyright © 2016年 vimfung. All rights reserved.
//

import UIKit
import LuaScriptCore_iOS_Swift

class ViewController: UIViewController {
    
    var _context : LuaContext = LuaContext();
    var _hasRegMethod : Bool = false;
    
    override func viewDidLoad() {
        
        _context.onException { (msg) in
            print("lua exception = \(msg ?? "")");
        }
        
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
                info["deviceName"] = UIDevice.current.name;
                info["deviceModel"] = UIDevice.current.model;
                info["systemName"] = UIDevice.current.systemName;
                info["systemVersion"] = UIDevice.current.systemVersion;
                
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
        _ = _context.evalScript(script: "LogModule:writeLog('Hello Lua Module!');");
    }
    
    
    /// 注册类按钮点击
    ///
    /// - Parameter sender: 事件对象
    @IBAction func regClassButtonClickedHandler(_ sender: Any)
    {
        _ = _context.evalScript(filePath: "test.lua");
    }
    
    
    /// 导入原生类型按钮点击
    ///
    /// - Parameter sender: 事件对象
    @IBAction func importNativeClassButtonClickedHandler(_ sender: Any)
    {
        _ = _context.evalScript(script: "local Data = LSCTNativeData; print(Data); local d = Data(); print(d); d.dataId = 'xxxx'; print(d.dataId); d:setData('xxx','testKey'); print(d:getData('testKey'));");
    }
    
}

