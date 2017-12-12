//
//  LuaValue.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 16/11/30.
//  Copyright © 2016年 vimfung. All rights reserved.
//

/// Lua值类型
///
/// - Nil: 空类型
/// - Number: 数值类型
/// - Boolean: 布尔类型
/// - String: 字符串类型
/// - Array: 数组类型
/// - Map: 字典类型
/// - Ptr: 指针类型
/// - Object: 对象类型
/// - Integer: 整型
/// - Data: 二进制数组类型
/// - Function: 方法类型
/// - Tuple: 元组
/// - Class: 类型
public enum LuaValueType : Int
{
    case Nil = 0
    case Number = 1
    case Boolean = 2
    case String = 3
    case Array = 4
    case Map = 5
    case Ptr = 6
    case Object = 7
    case Integer = 8
    case Data = 9
    case Function = 10
    case Tuple = 11
    case Class = 12
}


/// Lua值对象
public class LuaValue: NSObject
{
    private var _rawValue : LSCValue;
    
    /// 初始化
    public override init()
    {
        _rawValue = LSCValue.nil();
    }
    
    /// 初始化
    ///
    /// - Parameter intValue: 整型值
    public init(intValue : Int)
    {
        _rawValue = LSCValue.integerValue(intValue);
    }
    
    /// 初始化
    ///
    /// - Parameter doubleValue: 双精度浮点型
    public init(doubleValue : Double)
    {
        _rawValue = LSCValue.number(NSNumber(value: doubleValue));
    }
    
    /// 初始化
    ///
    /// - Parameter booleanValue: 布尔值
    public init(booleanValue : Bool)
    {
        _rawValue = LSCValue.booleanValue(booleanValue);
    }
    
    /// 初始化
    ///
    /// - Parameter stringValue: 字符串
    public init(stringValue : String)
    {
        _rawValue = LSCValue.stringValue(stringValue);
    }
    
    /// 初始化
    ///
    /// - Parameter arrayValue: 数组
    public init(arrayValue : Array<Any>)
    {
        _rawValue = LSCValue.arrayValue(arrayValue);
    }
    
    /// 初始化
    ///
    /// - Parameter dataValue: 二进制数据
    public init(dataValue : Data)
    {
        _rawValue = LSCValue.dataValue(dataValue);
    }
    
    /// 初始化
    ///
    /// - Parameter dictionaryValue: 字典
    public init(dictionaryValue : Dictionary<AnyHashable, Any>)
    {
        _rawValue = LSCValue.dictionaryValue(dictionaryValue);
    }
    
    /// 初始化
    ///
    /// - Parameter pointerValue: 指针
    public init(pointerValue : LuaPointer)
    {
        _rawValue = LSCValue.pointerValue(pointerValue._rawPointer);
    }
    
    /// 初始化
    ///
    /// - Parameter functionValue: 方法
    public init(functionValue : LuaFunction)
    {
        _rawValue = LSCValue.functionValue(functionValue._rawFunction);
    }
    
    /// 初始化
    ///
    /// - Parameter tupleValue: 元组
    public init(tupleValue : LuaTuple)
    {
        _rawValue = LSCValue.tupleValue(tupleValue._rawTuple);
    }
    
    
    /// 初始化
    ///
    /// - Parameter typeValue: 类型描述
    public init(typeValue : LuaExportTypeDescriptor)
    {
        _rawValue = LSCValue.typeValue(typeValue);
    }
    
    /// 初始化
    ///
    /// - Parameter objectValue: 对象
    public init(objectValue : AnyObject)
    {
        _rawValue = LSCValue.objectValue(objectValue);
    }
    
    
    
    /// 初始化
    ///
    /// - Parameter rawValue: 原始的LSCValue对象
    public init (rawValue : LSCValue)
    {
        _rawValue = rawValue;
    }
    
    /// 值类型
    public var type : LuaValueType
    {
        get
        {
            return LuaValueType(rawValue: Int(_rawValue.valueType.rawValue))!;
        }
    }
    
    /// 转换为整型值
    public var intValue : Int
    {
        get
        {
            return _rawValue.toInteger();
        }
    }
    
    /// 转换为浮点型值
    public var doubleValue : Double
    {
        get
        {
            return _rawValue.toDouble();
        }
    }
    
    /// 转换为布尔值
    public var booleanValue : Bool
    {
        get
        {
            return _rawValue.toBoolean();
        }
    }
    
    /// 转换为字符串值
    public var stringValue : String
    {
        get
        {
            return _rawValue.toString();
        }
    }
    
    /// 转换为数组
    public var arrayValue : Array<Any>
    {
        get
        {
            return _rawValue.toArray();
        }
    }
    
    /// 转换为字典
    public var dictionaryValue : Dictionary<AnyHashable , Any>
    {
        get
        {
            return _rawValue.toDictionary();
        }
    }
    
    /// 转换为对象
    public var objectValue : Any
    {
        get
        {
            return _rawValue.toObject();
        }
    }
    
    /// 转换为二进制数据
    public var dataValue : Data
    {
        get
        {
            return _rawValue.toData();
        }
    }
    
    /// 转换为方法对象
    public var functionValue : LuaFunction
    {
        get
        {
            return LuaFunction(rawFunction:_rawValue.toFunction());
        }
    }
    
    /// 转换为指针对象
    public var pointerValue : LuaPointer
    {
        get
        {
            return LuaPointer(rawPointer: _rawValue.toPointer());
        }
    }
    
    
    /// 转换为类型描述
    public var typeValue : LuaExportTypeDescriptor
    {
        get
        {
            return _rawValue.toType();
        }
    }
    
    /// 获取原始的Value值
    public var rawValue : LSCValue
    {
        get
        {
            return _rawValue;
        }
    }
}
