//
//  LuaClassImport.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/4/1.
//  Copyright © 2017年 vimfung. All rights reserved.
//

/// 类型导入
public class LuaClassImport: LSCClassImport
{
    /// 设置包含的导出原生类型列表
    ///
    /// - Parameters:
    ///   - classes: 类型列表
    ///   - context: 上下文对象
    public class func setInculdesClasses(classes : [AnyClass]!, context : LuaContext!) -> Void
    {
        super.setInculdesClasses(classes, with:context.getRawContext());
    }
}
