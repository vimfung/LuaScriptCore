//
//  LSCEncoding.swift
//  Sample
//
//  Created by 冯鸿杰 on 2018/8/21.
//  Copyright © 2018年 vimfung. All rights reserved.
//

#if os(iOS)

import Foundation
import LuaScriptCore_iOS_Swift

#elseif os(OSX)

import Cocoa
import LuaScriptCore_OSX_Swift

#endif

@objc(LSCEncoding)
class LSCEncoding : NSObject, LuaExportType
{
    /// URL编码
    ///
    /// - Parameter text: 需要编码的文本
    /// - Returns: 编码后的文本
    @objc static func urlEncode(text : String) -> String?
    {
        return text.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed);
    }
    
    /// URL解码
    ///
    /// - Parameter text: 需要解码的文本
    /// - Returns: 解码后的文本
    @objc static func urlDecode(text : String) -> String?
    {
        return text.removingPercentEncoding;
    }
}
