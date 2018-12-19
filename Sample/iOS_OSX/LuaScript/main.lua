local json = require("dkjson");

local tbl = getDeviceInfo();

local str = json.encode (tbl, { indent = true })

print (str)
local nowTime = "当前时间:" .. os.date("%c")
print (nowTime)
sleepx(3)
nowTime = "当前时间:" .. os.date("%c")
print (nowTime)

