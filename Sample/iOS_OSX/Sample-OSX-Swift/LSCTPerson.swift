//
//  LSCTPerson.swift
//  Sample
//
//  Created by 冯鸿杰 on 16/12/1.
//  Copyright © 2016年 vimfung. All rights reserved.
//

import LuaScriptCore_OSX_Swift

class LSCTPerson: NSObject, LuaExportType
{
    @objc var name : String? = nil;
    
    @objc func speak() -> Void
    {
        NSLog("%@ speak", name ?? "noname");
    }
    
    @objc func walk() -> Void
    {
        NSLog("%@ walk", name ?? "noname");
    }
    
}
