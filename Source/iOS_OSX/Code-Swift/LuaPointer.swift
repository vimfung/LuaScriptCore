//
//  LuaPointer.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/30.
//  Copyright © 2016年 vimfung. All rights reserved.
//

/// Lua的指针对象
public class LuaPointer: NSObject
{
    var _rawPointer : LSCPointer;
    
    /// 初始化
    public override init()
    {
        _rawPointer = LSCPointer();
    }
    
    /// 初始化
    ///
    /// - Parameter pointer: 原始指针
    public init(pointer : UnsafeRawPointer)
    {
        _rawPointer = LSCPointer(ptr: pointer);
    }
    
    
    /// 初始化
    ///
    /// - Parameter userdata: 原始的Userdata引用
    public init(userdata : LSCUserdataRef)
    {
        _rawPointer = LSCPointer(userdata: userdata);
    }
    
    /// 初始化
    ///
    /// - Parameter rawPointer: 原始的LSCPointer对象
    public init(rawPointer : LSCPointer)
    {
        _rawPointer = rawPointer;
    }
}
