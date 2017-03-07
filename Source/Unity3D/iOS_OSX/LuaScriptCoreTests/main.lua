function add (a, b)

    print("result = "..tostring(a * b));
    return a * b;

end

function testTuple()
	-- body
	print('-------- call testTuple');
	return "Hello", 2017, "World"
end
