//
//  LuaTuple.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/1/19.
//  Copyright © 2017年 vimfung. All rights reserved.
//

public class LuaTuple: NSObject
{
    var _rawTuple : LSCTuple;
    
    /// 初始化
    public override init ()
    {
        _rawTuple = LSCTuple();
    }
    
    /// 初始化
    ///
    /// - Parameter rawTuple: OC中的Lua元组对象实例
    public init(rawTuple : LSCTuple)
    {
        _rawTuple = rawTuple;
    }
    
    /// 获取元组数量
    public func count() -> Int
    {
        return _rawTuple.count;
    }
    
    /// 添加返回值
    ///
    /// - Parameter value: 返回值
    public func addReturnValue (value : Any)
    {
        _rawTuple.addReturnValue(value);
    }
    
    /// 获取返回值
    ///
    /// - Parameter index: 索引
    /// - Returns: 返回值
    public func returnValueForIndex (index : Int) -> Any
    {
        return _rawTuple.returnValue(for: index);
    }
}
