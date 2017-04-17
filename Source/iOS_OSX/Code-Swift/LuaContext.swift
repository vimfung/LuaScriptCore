//
//  LuaContext.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/30.
//  Copyright © 2016年 vimfung. All rights reserved.
//

/// Lua上下文对象
public class LuaContext: NSObject
{
    private var _rawContext:LSCContext;
    
    
    /// 获取原始上下文对象
    ///
    /// - Returns: 原始上下文对象
    internal func getRawContext() -> LSCContext
    {
        return _rawContext;
    }
    
    
    /// 初始化LuaContext
    public override init()
    {
        _rawContext = LSCContext();
    }
    
    
    /// 添加搜索路径，对于不在应用主Bundle根目录的lua脚本如果需要require时，则需要指定其搜索路径。
    ///
    /// - Parameter path: 路径字符串
    public func addSearchPath(path : String) -> Void
    {
        _rawContext.addSearchPath(path);
    }
    
    
    /// 运行异常时触发
    ///
    /// - Parameter handler: 事件处理器
    public func onException (handler : @escaping (String?) -> Void) -> Void
    {
        _rawContext.onException { (message : String?) in
            
            handler (message);
            
        }
    }
    
    /// 设置全局变量
    ///
    /// - Parameters:
    ///   - name: 变量名称
    ///   - value: 变量值
    public func setGlobal (name : String, value : LuaValue) -> Void
    {
        _rawContext.setGlobalWith(value.rawValue, forName: name);
    }
    
    /// 获取全局变量
    ///
    /// - Parameter name: 变量名称
    /// - Returns: 变量值
    public func getGlobal (name : String) -> LuaValue
    {
        let value : LSCValue =  _rawContext.getGlobalForName(name);
        return LuaValue(rawValue: value);
    }
    
    /// 解析脚本
    ///
    /// - Parameter script: 脚本字符串
    /// - Returns: 返回值对象
    public func evalScript (script : String) -> LuaValue
    {
        let retValue : LSCValue = _rawContext.evalScript(from: script);
        return LuaValue(rawValue: retValue as LSCValue);
    }
    
    /// 解析脚本
    ///
    /// - Parameter filePath: 文件路径
    /// - Returns: 返回值对象
    public func evalScript (filePath : String) -> LuaValue
    {
        let retValue : LSCValue = _rawContext.evalScript(fromFile: filePath);
        return LuaValue(rawValue: retValue as LSCValue);
    }
    
    
    /// 调用方法
    ///
    /// - Parameters:
    ///   - methodName: 方法名称
    ///   - arguments: 参数列表
    /// - Returns: 返回值对象
    public func callMethod (methodName : String, arguments : Array<LuaValue>) -> LuaValue
    {
        var args : Array<LSCValue> = Array<LSCValue>();
        for item in arguments {
            args.append(item.rawValue);
        }
        
        let retValue : LSCValue = _rawContext.callMethod(withName: methodName, arguments: args);
        return LuaValue(rawValue: retValue as LSCValue);
        
    }
    
    /// 注册方法
    ///
    /// - Parameters:
    ///   - methodName: 方法名称
    ///   - block: 方法处理
    public func registerMethod (methodName : String, block: @escaping (Array<LuaValue>) -> LuaValue) -> Void
    {
        _rawContext.registerMethod(withName: methodName, block: {(arguments : Array<LSCValue>?) in
            
            var args : Array<LuaValue> = Array<LuaValue> ();
            if ((arguments) != nil)
            {
                for item in arguments! {
                    args.append(LuaValue(rawValue: item));
                }
            }
            
            let retValue : LuaValue? = block (args);
            if ((retValue) != nil)
            {
                return retValue?.rawValue;
            }
            
            return nil;
            
        });
    }
    
    /// 注册模块
    ///
    /// - Parameter moduleClass: 模块类型，必须继承于LSCModule
    public func registerModule(moduleClass : AnyClass) -> Void
    {
        _rawContext.registerModule(with: moduleClass);
    }
    
    
    /// 反注册模块
    ///
    /// - Parameter moduleClass: 模块类型，必须继承于LSCModule
    public func unregisterModule(moduleClass : AnyClass) -> Void
    {
        _rawContext.unregisterModule(with: moduleClass);
    }
}
