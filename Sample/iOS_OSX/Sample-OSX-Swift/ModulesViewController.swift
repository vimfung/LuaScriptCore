//
//  ModulesViewController.swift
//  Sample-OSX-Swift
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

import Cocoa
import LuaScriptCore_OSX_Swift

class ModulesViewController: NSViewController, NSOutlineViewDataSource, NSOutlineViewDelegate
{
    @IBOutlet weak var outlineView: NSOutlineView!
    
    let context : LuaContext = LuaContext();
    
    override func viewDidLoad() {
        
        context.onException { (msg) in
            print("lua exception = \(msg ?? "")");
        }
        
        _ = context.evalScript(filePath: "Encoding-Sample.lua");
        _ = context.evalScript(filePath: "Path-Sample.lua");
        _ = context.evalScript(filePath: "HTTP-Sample.lua");
        _ = context.evalScript(filePath: "Thread-Sample.lua");
        _ = context.evalScript(filePath: "Crypto-Sample.lua");
    }
    
    func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int
    {
        let strItem : String? = item as? String;
        if strItem == nil
        {
            return 5;
        }
        else if (strItem == "Encoding")
        {
            return 8;
        }
        else if (strItem == "Path")
        {
            return 5;
        }
        else if (strItem == "HTTP")
        {
            return 4;
        }
        else if (strItem == "Thread")
        {
            return 1;
        }
        else if (strItem == "Crypto")
        {
            return 4;
        }
        
        return 0;
    }
    
    func outlineView(_ outlineView: NSOutlineView, child index: Int, ofItem item: Any?) -> Any
    {
        let strItem : String? = item as? String;
        if strItem == nil
        {
            switch (index)
            {
            case 0:
                return "Encoding";
            case 1:
                return "Path";
            case 2:
                return "HTTP";
            case 3:
                return "Thread";
            case 4:
                return "Crypto";
            default:
                return "";
            }
        }
        else if strItem == "Encoding"
        {
            switch (index)
            {
            case 0:
                return "Url Encode";
            case 1:
                return "Url Decode";
            case 2:
                return "Base64 Encode";
            case 3:
                return "Base64 Decode";
            case 4:
                return "JSON Encode";
            case 5:
                return "JSON Decode";
            case 6:
                return "Hex Encode";
            case 7:
                return "Hex Decode";
            default:
                return "";
            }
        }
        else if strItem == "Path"
        {
            switch (index)
            {
            case 0:
                return "App Path";
            case 1:
                return "Home Path";
            case 2:
                return "Documents Path";
            case 3:
                return "Caches Path";
            case 4:
                return "Tmp Path";
            default:
                return "";
            }
        }
        else if strItem == "HTTP"
        {
            switch (index)
            {
            case 0:
                return "GET Request";
            case 1:
                return "POST Request";
            case 2:
                return "Upload File";
            case 3:
                return "Download File";
            default:
                return "";
            }
        }
        else if strItem == "Thread"
        {
            switch (index)
            {
            case 0:
                return "Run Thread";
            default:
                return "";
            }
        }
        else if strItem == "Crypto"
        {
            switch (index)
            {
            case 0:
                return "MD5";
            case 1:
                return "SHA1";
            case 2:
                return "HMAC-MD5";
            case 3:
                return "HMAC-SHA1";
            default:
                return "";
            }
        }
        
        return "";
    }
    
    func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool
    {
        let strItem : String? = item as? String;
        if (strItem == nil
            || strItem == "Encoding"
            || strItem == "Path"
            || strItem == "HTTP"
            || strItem == "Thread"
            || strItem == "Crypto")
        {
            return true;
        }
        
        return false;
    }
    
    func outlineView(_ outlineView: NSOutlineView, objectValueFor tableColumn: NSTableColumn?, byItem item: Any?) -> Any?
    {
        return item;
    }
    
    func outlineViewSelectionDidChange(_ notification: Notification)
    {
        let item : String? = outlineView.item(atRow: outlineView.selectedRow) as? String;
        if item == "Url Encode"
        {
            _ = context.evalScript(script: "Encoding_Sample_urlEncode()");
        }
        else if item == "Url Decode"
        {
            _ = context.evalScript(script: "Encoding_Sample_urlDecode()");
        }
        else if item == "Base64 Encode"
        {
            _ = context.evalScript(script: "Encoding_Sample_base64Encode()");
        }
        else if item == "Base64 Decode"
        {
            _ = context.evalScript(script: "Encoding_Sample_base64Decode()");
        }
        else if item == "JSON Encode"
        {
            _ = context.evalScript(script: "Encoding_Sample_jsonEndode()");
        }
        else if item == "JSON Decode"
        {
            _ = context.evalScript(script: "Encoding_Sample_jsonDecode()");
        }
        else if item == "Hex Encode"
        {
            _ = context.evalScript(script: "Encoding_Sample_hexEncode()");
        }
        else if item == "Hex Decode"
        {
            _ = context.evalScript(script: "Encoding_Sample_hexDecode()");
        }
        else if item == "App Path"
        {
            _ = context.evalScript(script: "Path_Sample_appPath()");
        }
        else if item == "Home Path"
        {
            _ = context.evalScript(script: "Path_Sample_homePath()");
        }
        else if item == "Documents Path"
        {
            _ = context.evalScript(script: "Path_Sample_docsPath()");
        }
        else if item == "Caches Path"
        {
            _ = context.evalScript(script: "Path_Sample_cachesPath()");
        }
        else if item == "Tmp Path"
        {
            _ = context.evalScript(script: "Path_Sample_tmpPath()");
        }
        else if item == "GET Request"
        {
            _ = context.evalScript(script: "HTTP_Sample_get()");
        }
        else if item == "POST Request"
        {
            _ = context.evalScript(script: "HTTP_Sample_post()");
        }
        else if item == "Upload File"
        {
            _ = context.evalScript(script: "HTTP_Sample_upload()");
        }
        else if item == "Download File"
        {
            _ = context.evalScript(script: "HTTP_Sample_download()");
        }
        else if item == "Run Thread"
        {
            _ = context.evalScript(script: "Thread_Sample_run()");
        }
        else if item == "MD5"
        {
            _ = context.evalScript(script: "Crypto_Sample_md5()");
        }
        else if item == "SHA1"
        {
            _ = context.evalScript(script: "Crypto_Sample_sha1()");
        }
        else if item == "HMAC-MD5"
        {
            _ = context.evalScript(script: "Crypto_Sample_hmacMD5()");
        }
        else if item == "HMAC-SHA1"
        {
            _ = context.evalScript(script: "Crypto_Sample_hmacSHA1()");
        }
    }
}
