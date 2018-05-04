Person.prototype.age = {
	
	get = function (self)
	    print ("get age = ", self._age);
		return self._age;
	end,
	set = function (self, value)
	    print ("set age = ", value);
		self._age = value;
	end
	
};

local p = Person();  p.age = 12; print (p.age);