require "threads"
local thread = threads.thread;

function thread_hello(t)
	local t = thread.new(t);
	print("hello from Thread: "..t.id);

	--thread.sleep(2,0); -- sleep for 2 seconds
	print(string.format("thread: %d has finished ...", t.id));

	--thread.complete(t);
end

local function main(...)
	local ret, stack_size, stack_addr;
	local t0 = thread.new();

	ret, stack_size = thread.attr.stacksize(t0); 
	ret, stack_addr = thread.attr.stackaddr(t0);

	print("Stack Size: ", stack_size, ret);
	print(string.format("Stack Addr: %s ret: %d", tostring(stack_addr) or 0, ret));
	print("\n----- Start Tests -----\n");

	thread.attr.show(t0);
	thread.create(t0, "thread_hello");
	thread.sleep(1)
	thread.join(t0); -- wait on thread t0 then free resources

	print("\n----- Tests Complete -----\n");
end

main(...)