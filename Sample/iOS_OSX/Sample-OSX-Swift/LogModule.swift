//
//  LogModule.swift
//  Sample
//
//  Created by 冯鸿杰 on 16/12/1.
//  Copyright © 2016年 vimfung. All rights reserved.
//

import LuaScriptCore_OSX_Swift

class LogModule: LSCModule
{
    static func writeLog(message : String) -> Void
    {
        NSLog("** message = %@", message);
    }
}
