//
//  LuaScriptCore_iOS_SwiftTests.swift
//  LuaScriptCore-iOS-SwiftTests
//
//  Created by 冯鸿杰 on 16/11/30.
//  Copyright © 2016年 vimfung. All rights reserved.
//

import XCTest
@testable import LuaScriptCore_iOS_Swift

class LuaScriptCore_iOS_SwiftTests: XCTestCase {
    
    private var _context : LuaContext?;
    
    override func setUp()
    {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
        _context = LuaContext();
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testGetSetGlobal()
    {
        _context?.setGlobal(name: "abc", value: LuaValue(stringValue: "Hello World"));
        let value : LuaValue = (_context?.getGlobal(name: "abc"))!;
        NSLog("value = %@", value.stringValue);
    }
    
    func testClassImport()
    {
        LSCClassImport.setInculdesClasses([Person.classForCoder()]);
        _context?.registerModule(moduleClass: LSCClassImport.classForCoder());
        
        _ = _context?.evalScript(script: "local Person = ClassImport('LuaScriptCore_iOS_SwiftTests.Person'); local p = Person.createPerson(); p:setName('vim'); p:speak('Hello World!');");
    }
    
}
