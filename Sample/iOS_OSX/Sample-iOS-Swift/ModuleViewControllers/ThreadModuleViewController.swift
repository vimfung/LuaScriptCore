//
//  ThreadModuleViewController.swift
//  Sample-iOS-Swift
//
//  Created by 冯鸿杰 on 2019/1/2.
//  Copyright © 2019年 vimfung. All rights reserved.
//

import UIKit
import LuaScriptCore_iOS_Swift

class ThreadModuleViewController: UITableViewController {

    let context : LuaContext = LuaContext();
    
    override func viewDidLoad() {
        super.viewDidLoad()

        context.onException { (msg) in
            print("lua exception = \(msg!)");
        }
        
        _ = context.evalScript(filePath: "Thread-Sample.lua");
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        
        switch indexPath.row
        {
        case 0:
            _ = context.evalScript(script: "Thread_Sample_run()");
        default:
            break;
        }
    }

}
