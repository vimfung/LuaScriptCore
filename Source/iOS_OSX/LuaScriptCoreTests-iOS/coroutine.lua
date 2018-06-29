function Resume(func)
    coroutine.resume(func)
end

function Test()

    logic = coroutine.create(function()
        while true do
            value = GetValue() --返回function?
            print ("------ value = ", value);
            r, g, b = GetPixel(1000, 1200) --返回1000, 1200
            print ("------ r, g, b = ", r, g, b);
            coroutine.yield()
        end
    end)

    Resume(logic);

end

function TestModuleFunc()
    
    local tfunc = coroutine.create(function()

        local str = TestModule();
        print (str);

        coroutine.yield()

    end)

    coroutine.resume(tfunc);

end

function TestClassFunc()

    local tfunc = coroutine.create(function()

        print ("+++++++++++++");

        local p = Person();
        print (p);

        p:speak("i am vim");

        p = Person:createPerson();
        print (p);

        p.name = "vim";
        print (p.name);

        Person:printPersonName(p);

        coroutine.yield()

    end)

    coroutine.resume(tfunc);

end

function TestClassImportFunc()

    local tfunc = coroutine.create(function()

        print(Person, NativePerson); 
        local p = NativePerson:createPerson(); 
        print(p); 
        p.name = "abc"
        p:speak('Hello World!');

        coroutine.yield()

    end)

    coroutine.resume(tfunc);

end

Test();
TestModuleFunc();
TestClassFunc();
TestClassImportFunc();
