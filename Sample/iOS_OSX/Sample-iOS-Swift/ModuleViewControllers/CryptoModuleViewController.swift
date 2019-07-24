//
//  CryptoModuleViewController.swift
//  Sample-iOS-Swift
//
//  Created by 冯鸿杰 on 2019/6/16.
//  Copyright © 2019年 vimfung. All rights reserved.
//

import UIKit
import LuaScriptCore_iOS_Swift

class CryptoModuleViewController: UITableViewController {

    let context : LuaContext = LuaContext();
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        context.onException { (msg) in
            print("lua exception = \(msg!)");
        }
        
        _ = context.evalScript(filePath: "Crypto-Sample.lua");
    }
    
    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        
        switch indexPath.row
        {
        case 0:
            _ = context.evalScript(script: "Crypto_Sample_md5()");
        case 1:
            _ = context.evalScript(script: "Crypto_Sample_sha1()");
        case 2:
            _ = context.evalScript(script: "Crypto_Sample_hmacMD5()");
        case 3:
            _ = context.evalScript(script: "Crypto_Sample_hmacSHA1()");
        default:
            break;
        }
    }
    
}
