Class = {};
Class.__index = function (table, key)
	return "Hello";
end;

Class.__newindex = function (table, key, value)
	print (key, value);
end

function Class.create() 

  local instance = {};
  instance.a = 1;

  setmetatable(instance, Class);
  return instance;  

end

local obj = Class.create();
obj.a = 1111;
print (obj.a);