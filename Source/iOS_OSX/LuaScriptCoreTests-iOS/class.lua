Class = {};
Class.__index = Class

Class.objCount = 0;
Class.name = "Object";

local Class_Constructor = {};
Class_Constructor.__call = function (type)
	
	Class.objCount = Class.objCount + 1;

  	local instance = {};
  	instance.class = type;
  	setmetatable(instance, type.prototype);
  	return instance;  

end
setmetatable(Class, Class_Constructor);
Class.__call = Class_Constructor.__call;

-- function Class:create() 

-- 	Class.objCount = Class.objCount + 1;

--   	local instance = {};
--   	instance.class = self;
--   	setmetatable(instance, self.prototype);
--   	return instance;  

-- end

function Class:subclass(typeName)
	
	-- 以传入类型名称作为全局变量名称创建table
	_G[typeName] = {};

	-- 设置元方法__index,并绑定父级类型作为元表
	local subtype = _G[typeName];

	subtype.name = typeName;
	subtype.super = self;
	subtype.__call = Class_Constructor.__call;
	subtype.__index = subtype;
	setmetatable(subtype, self);

	-- 创建prototype并绑定父类prototype作为元表
	subtype.prototype = {};
	subtype.prototype.__index = subtype.prototype;
	subtype.prototype.__gc = self.prototype.__gc;
	subtype.prototype.__tostring = self.prototype.__tostring;
	setmetatable(subtype.prototype, self.prototype);

	return subtype;

end

Class.prototype = {};
Class.prototype.__index = Class.prototype;
Class.prototype.__gc = function (instance)
	print(instance, "destroy");
end

Class.prototype.tag = 999;
Class.prototype.__tostring = function (instance)
	
	return "[" .. instance.class.name .." object]";

end


function Class.prototype:add(a, b)
	-- body
	return a + b;
end

Class:subclass("Child");

function Child.prototype:sumSquare(a, b)
	-- body
	local sum = Child.super.prototype.add(self, a, b);
	return sum ^ 2;
end

local c = Child();
print (c:sumSquare(2, 2));

local obj = Class();
print (obj);
-- print (obj.tag);
-- obj.tag = 999;
-- obj = nil;


-- local obj2 = Class:create();
-- print (obj2.tag);
-- obj2 = nil;

-- print (Class.objCount);

Class:subclass("Person")
Person.prototype.name = "vim";

-- function Person.prototype:output(msg)

-- 	msg = self.name .. ":" .. msg;
-- 	Person.super.prototype.output(self, msg);

-- 	-- self.super.output(self, msg);
-- end

local p = Person();
print (p);
print (p.name);
-- p:output("Hello World!");
-- p = nil;

-- Person:subclass("Chinese");
-- Chinese.prototype.skin = "yellow";

-- function Chinese.prototype:output(msg)
-- 	-- body
-- 	msg = "(" .. self.skin.. ")" .. msg;
-- 	Chinese.super.prototype.output(self, msg);
-- end

-- local ch = Chinese:create();
-- print (ch.name, ch.skin);
-- ch:output("Hello World!");
-- ch = nil;

collectgarbage("collect")