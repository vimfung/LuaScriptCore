function add (a, b)

    return a+b;

end

function getFunc()

	return function (a, b)
		
		return a * b;

	end

end

function printPointer(p)
	print(p);
end

function testTuple()
	-- body
	return "Hello", 2017, "World"
end