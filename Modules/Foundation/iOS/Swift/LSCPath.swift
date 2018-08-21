//
//  LSCPath.swift
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

@objc(LSCPath)
class LSCPath : NSObject, LuaExportType
{
    
    /// 获取应用目录
    ///
    /// - Returns: 路径信息
    @objc static func appPath() -> String?
    {
        return Bundle.main.resourcePath;
    }
    
    /// 获取应用沙箱根目录
    ///
    /// - Returns: 路径信息
    @objc static func homePath() -> String?
    {
        return NSHomeDirectory();
    }
    
    /// 获取文档目录
    ///
    /// - Returns: 路径信息
    @objc static func docsPath() -> String?
    {
        return NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true).first;
    }
    
    /// 获取缓存目录
    ///
    /// - Returns: 路径信息
    @objc static func cachesPath() -> String?
    {
        return NSSearchPathForDirectoriesInDomains(.cachesDirectory, .userDomainMask, true).first;
    }
    
    /// 获取临时目录
    ///
    /// - Returns: 路径信息
    @objc static func tmpPath() -> String?
    {
        return NSTemporaryDirectory();
    }
}
