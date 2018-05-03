--
-- Created by IntelliJ IDEA.
-- User: vimfung
-- Date: 2017/7/5
-- Time: 下午4:38
-- To change this template use File | Settings | File Templates.
--


function Resume(func)
    coroutine.resume(func)
end

function Test()

    logic = coroutine.create(function()
        while true do
            local value = GetValue() --返回function?
            print ("------ value = ", value);
            local r, g, b = GetPixel(1000, 1200) --返回1000, 1200
            print ("------ r, g, b = ", r, g, b);
            coroutine.yield()
        end
    end)

    Resume(logic);

end

function TestModuleFunc()

    local tfunc = coroutine.create(function()

        print (LogModule:test({111, 2222}));

        coroutine.yield()

    end)

    coroutine.resume(tfunc);

end

function TestClassFunc()

    local tfunc = coroutine.create(function()

        print ("+++++++++++++");

        local p = Person:create();
        print (p);
        p.name = "abc";

        p:speak();

        local p = Person:createPerson();
        print (p);

        p.name = "vim";
        print (p.name);
        p:speak();

        Person:printPerson(p);

        coroutine.yield()

    end)

    coroutine.resume(tfunc);

end

function TestClassImportFunc()

    local tfunc = coroutine.create(function()

        print ("-------------");

        print(Person, NativeData);
        local p = NativeData:createPerson();
        print(p);
        p.name = "abc";
        p:speak();

        coroutine.yield()

    end)

    coroutine.resume(tfunc);

end

Test();
TestModuleFunc();
TestClassFunc();
TestClassImportFunc();
