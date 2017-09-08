function LSCTPerson.prototype:destroy ()

    print("LSCTPerson destroy");

end

local person = LSCTPerson.create();
person:setName('vimfung');
person:walk();
person:speak();
