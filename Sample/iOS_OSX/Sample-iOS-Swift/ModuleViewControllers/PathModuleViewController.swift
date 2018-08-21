//
//  PathModuleViewController.swift
//  Sample-iOS-Swift
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

import UIKit
import LuaScriptCore_iOS_Swift

class PathModuleViewController: UITableViewController
{
    let context : LuaContext = LuaContext();
    
    override func viewDidLoad() {
        
        context.onException { (msg) in
            print("lua exception = \(msg!)");
        }
        
        _ = context.evalScript(filePath: "Path-Sample.lua");
    }
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        
        switch indexPath.row
        {
        case 0:
            _ = context.evalScript(script: "Path_Sample_appPath()");
        case 1:
            _ = context.evalScript(script: "Path_Sample_homePath()");
        case 2:
            _ = context.evalScript(script: "Path_Sample_docsPath()");
        case 3:
            _ = context.evalScript(script: "Path_Sample_cachesPath()");
        case 4:
            _ = context.evalScript(script: "Path_Sample_tmpPath()");
        default:
            break;
        }
    }
}
