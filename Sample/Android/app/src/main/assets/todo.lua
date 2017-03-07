function add (a, b)

    return a+b;

end

function makeList()
    -- body
    local list = {}
    for i=1,600 do
        local obj = {}
        obj.name = "test"
        table.insert(list, obj)
    end

    return list;
end

function getFunc()

    return function (a, b)

        print("------", a, b);
        return a * b;

    end

end