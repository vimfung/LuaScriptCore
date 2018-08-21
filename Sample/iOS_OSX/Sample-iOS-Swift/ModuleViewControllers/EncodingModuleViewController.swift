//
//  EncodingModuleViewController.swift
//  Sample-iOS-Swift
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

import UIKit
import LuaScriptCore_iOS_Swift

class EncodingModuleViewController: UITableViewController {

    let context : LuaContext = LuaContext();
    
    override func viewDidLoad() {
        
        context.onException { (msg) in
            print("lua exception = \(msg!)");
        }
        
        _ = context.evalScript(filePath: "Encoding-Sample.lua");
    }
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        
        switch indexPath.row
        {
        case 0:
           _ = context.evalScript(script: "Encoding_Sample_urlEncode()");
        case 1:
           _ = context.evalScript(script: "Encoding_Sample_urlDecode()");
        default:
            break;
        }
    }
    
}
