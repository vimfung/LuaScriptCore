Person.prototype.age = {

	get = function (self)
		print("*********get age", self._age);
		return self._age;
	end,
	set = function (self, value)
		print("-------set age", self._age);
		self._age = value;
		print("+++++++set age", self._age);
	end

};

local p = Person();  p.age = 12; print (p.age);

Object:subclass("Child");

Child.prototype.name = {
    get = function (self)
    	print ("+++++++++ get name");
    	return self._name;
    end,
    set = function (self, value)
    	print ("+++++++++ set name");
    	self._name = value;
    end
};

local c = Child(); c.name = "vim"; print (c.name);
