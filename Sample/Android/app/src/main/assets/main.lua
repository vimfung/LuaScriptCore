local json = require("dkjson");

local tbl = getDeviceInfo();

local str = json.encode (tbl, { indent = true })

print (str)
