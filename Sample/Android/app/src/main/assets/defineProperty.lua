Person.prototype.age = {
	
	get = function (self)
		return self._age;
	end,
	set = function (self, value)
		self._age = value;
	end
	
};

local p = Person();  p.age = 12; print (p.age);