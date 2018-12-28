//
//  LuaThread.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2018/12/28.
//  Copyright © 2018年 vimfung. All rights reserved.
//

import Foundation


/// 线程
public class LuaThread: NSObject
{
    /// 原始的线程对象
    var rawThread : LSCThread;
    /// 上下文对象
    var _context : LuaContext;
    
    /// 上下文对象
    public var context : LuaContext
    {
        get
        {
            return _context;
        }
    }
    
    /// 线程是否执行完成
    public var finished : Bool
    {
        get
        {
            return rawThread.finished;
        }
    }
    
    
    /// 初始化
    ///
    /// - Parameters:
    ///   - context: 上下文对象
    ///   - handler: 线程处理器
    public init(context : LuaContext, handler : @escaping (Array<LuaValue>?) -> LuaValue?)
    {
        _context = context;
        rawThread = LSCThread(context: context.rawContext) { (arguments) -> LSCValue? in
            
            var args : Array<LuaValue> = Array<LuaValue> ();
            if ((arguments) != nil)
            {
                for item in arguments!
                {
                    args.append(LuaValue(rawValue: item));
                }
            }
            
            let retValue : LuaValue? = handler (args);
            if ((retValue) != nil)
            {
                return retValue?.rawValue;
            }
            
            return nil;
        };
    }
    
    /// 恢复线程
    ///
    /// - Parameter arguments: 参数
    public func resume( arguments : Array<LuaValue>?) -> Void
    {
        var args : Array<LSCValue> = Array<LSCValue>();
        for item in arguments ?? [] {
            args.append(item.rawValue);
        }
        
        rawThread.resume(withArguments: args);
    }
    
    /// 挂起线程
    ///
    /// - Parameter result: 返回结果
    public func yield(result:LuaValue) -> Void
    {
        rawThread.yield(withResult: result.rawValue);
    }
}
