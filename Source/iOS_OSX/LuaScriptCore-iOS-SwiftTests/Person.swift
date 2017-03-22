//
//  Person.swift
//  LuaScriptCore
//
//  Created by 冯鸿杰 on 17/3/22.
//  Copyright © 2017年 vimfung. All rights reserved.
//

import UIKit

class Person: NSObject {
    
    var _name : String = "";
    
    static func createPerson() -> Person
    {
        return Person();
    }
    
    func setName(name : String) -> Void
    {
        _name = name;
    }
    
    func name() -> String
    {
        return _name;
    }
    
    func speak(text : String) -> Void
    {
        NSLog("%@ speak : %@", _name, text);
    }

}
