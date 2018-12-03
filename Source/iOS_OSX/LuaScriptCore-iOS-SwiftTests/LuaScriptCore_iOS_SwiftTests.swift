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
        _context = Env.defaultContext;
        _context?.onException(handler: { (str) in
            print("error \(String(describing: str))");
        })
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    
    /// 预期结果
    /**
     vimfung
     xxxxxxx
     */
    func testCreateObj()
    {
        _ = _context?.evalScript(script: "local p = Person(); print(p.name); local p2 = Person(\"xxxxxxx\"); print(p2.name);");
    }
    
    
    /// 预期结果
    /**
     value = Hello World
     */
    func testGetSetGlobal()
    {
        _context?.setGlobal(name: "abc", value: LuaValue(stringValue: "Hello World"));
        let value : LuaValue = (_context?.getGlobal(name: "abc"))!;
        NSLog("value = %@", value.stringValue);
    }
    
    
    /// 预期结果
    /**
     [Person type]
     [Person object<0x7fe42a31a538>]
     vim speak : Hello World!
    */
    func testClassImport()
    {
        _ = _context?.evalScript(script: "print(Person); local p = Person:createPerson(); print(p); p.name = 'vim'; p:speak('Hello World!');");
    }
    
    /// 预期结果
    /**
     test func
     -------------1
     test func
     -------------2
     */
    func testRetainAndRelease()
    {
        _ = _context?.evalScript(script: "local test = function() print('test func') end; test(); Person:retainHandler(test);");
        _ = _context?.evalScript(script: "print('-------------1'); Person:callHandler(); Person:releaseHandler();");
        _ = _context?.evalScript(script: "print('-------------2'); Person:callHandler();");
    }
    
    /// 预期结果
    /**
     test func
     -------------1
     test func
     -------------2
     */
    func testRetainAndRelease_2()
    {
        _ = _context?.evalScript(script: "local test = function() print('test func') end; test(); Person:retainHandler2(test);");
        _ = _context?.evalScript(script: "print('-------------1'); Person:callHandler2(); Person:releaseHandler2();");
        _ = _context?.evalScript(script: "print('-------------2'); Person:callHandler2();");
    }
    
    /// 预期结果
    /**
     [Person type]
     [Person object<0x7fc983107ab8>]
     vim speak : Hello World!
     */
    func testMultiThreadCall()
    {
        let exp = XCTestExpectation(description: "");
        
        let group = DispatchGroup();
        
        for _ in 0...100
        {
            group.enter();
            DispatchQueue.global().async(group: group, execute: DispatchWorkItem(block: {
                
                _ = self._context?.evalScript(script: "print(Person); local p = Person:createPerson(); print(p); p.name = 'vim'; p:speak('Hello World!');");
                
                group.leave();
                
            }));
        }
        
        group.notify(queue: DispatchQueue.main) {
            
            exp.fulfill();
            
        };
        
        self.wait(for: [exp], timeout: 20);
    
    }
    
}
