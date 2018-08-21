//
//  LSCHTTPFile.swift
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#if os(iOS)

import LuaScriptCore_iOS_Swift

#elseif os(OSX)

import LuaScriptCore_OSX_Swift

#endif

/// HTTP文件
@objc(LSCHTTPFile)
class LSCHTTPFile : NSObject, LuaExportType
{
    
    
    /// 路径
    @objc var path : String?;
    
    /// 文件类型
    var _mimeType : String?;
    @objc var mimeType : String
    {
        set
        {
            _mimeType = newValue;
        }
        get
        {
            if _mimeType == nil
            {
                return "application/octet-stream";
            }
            
            return _mimeType!;
        }
    }
    
    /// 传输编码
    @objc var transferEncoding : String?;
}
