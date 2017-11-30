//
//  Env.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 2017/5/10.
//  Copyright © 2017年 vimfung. All rights reserved.
//

import Foundation
import LuaScriptCore_iOS_Swift

class Env : NSObject
{
    static let defaultContext = setupContext();
    
    class func setupContext () -> LuaContext
    {
        let _context : LuaContext = LuaContext();
        return _context;
    }
}
