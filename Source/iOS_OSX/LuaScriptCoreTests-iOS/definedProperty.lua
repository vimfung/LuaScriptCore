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

local p = Person.create();  p.age = 12; print (p.age);
