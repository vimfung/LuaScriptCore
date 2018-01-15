function LSCTPerson.prototype:destroy ()

    print("LSCTPerson destroy");

end

local person = LSCTPerson.create();
person.name = "vimfung";
person:walk();
person:speak();
