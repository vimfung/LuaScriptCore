//
//  Env.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/10.
//  Copyright © 2017年 vimfung. All rights reserved.
//

import Foundation
import LuaScriptCore_iOS_Swift

private let _context : LuaContext = LuaContext()
class Env : NSObject
{
    class var defaultContext : LuaContext
    {
        get
        {
            return _context;
        }
    }
}
