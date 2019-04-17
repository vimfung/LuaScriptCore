url = "https://vimfung.github.io/LuaScriptCore/";

local json = require("dkjson");

local tbl = getDeviceInfo();

local str = json.encode (tbl, { indent = true })

print(url);

print(str);

function add (a, b)

    return a+b;

end

function printUrl(url)

    print (url);

end

function test()

local value = 256;
return value * 4, 1111, 'Hello';

end

return test();

