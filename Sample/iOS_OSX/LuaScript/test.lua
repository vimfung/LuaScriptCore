function LSCTPerson.prototype:destroy ()

    print("LSCTPerson destroy");

end

local person = LSCTPerson();
person.name = "vimfung";
person:walk();
person:speak();
