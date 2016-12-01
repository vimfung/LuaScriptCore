//
//  LSCTPerson.swift
//  Sample
//
//  Created by 冯鸿杰 on 16/12/1.
//  Copyright © 2016年 vimfung. All rights reserved.
//

import LuaScriptCore_iOS_Swift

class LSCTPerson: LSCObjectClass
{
    var name : String? = nil;
    
    func speak() -> Void
    {
        NSLog("%@ speak", name ?? "noname");
    }
    
    func walk() -> Void
    {
        NSLog("%@ walk", name ?? "noname");
    }
}
