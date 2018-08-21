//
//  LuaFunction.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/30.
//  Copyright © 2016年 vimfung. All rights reserved.
//

public typealias LuaFunction = LSCFunction;

extension LuaFunction
{
    /// 调用方法
    ///
    /// - Parameter arguments: 调用参数
    /// - Returns: 返回值, 如果返回值数量为1时，返回值类型是LuaValue， 如果数量>1时，则返回值类型是LuaTuple
    public func invoke (arguments : Array<LuaValue>) -> LuaValue
    {
        var args : Array<LSCValue> = Array<LSCValue>();
        for item in arguments {
            args.append(item.rawValue);
        }
        
        let retValue : LSCValue = self.invoke(withArguments: args);
        return LuaValue(rawValue: retValue as LSCValue);
    }
}
