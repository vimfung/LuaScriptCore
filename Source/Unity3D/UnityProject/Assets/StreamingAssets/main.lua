local json = require("dkjson");

local tbl = getDeviceInfo({1,2,3,4});

local str = json.encode (tbl, { indent = true })

print (str)

print (testReturnTuple());