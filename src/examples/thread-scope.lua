require "threads"
local thread = threads.thread;

function thread_hello(t)
	local t = thread.new(t);
	print("hello from Thread: "..t:id());
	local ret, scope = thread.attr.scope(t);
	print("Thread Scope: "..scope.."\nret: ".. ret.."\n");

	--thread.sleep(2,0); -- sleep for 2 seconds
	print(string.format("thread: %d has finished ...", t:id()));

	thread.complete(t);
end

local function main(arg)
	local ret, scope;
	local t0 = thread.new();

	ret = thread.attr.init(t0);
	ret = thread.attr.scope(t0, "system");
	ret, scope = thread.attr.scope(t0);

	print("Scope: ", scope, ret);
	print("\n----- Start Tests -----\n");

	ret = thread.create(t0, "thread_hello");
	ret = thread.join(t0); -- wait on thread t1 then free resources

	print("\n----- Tests Complete -----\n");
end

main(arg)