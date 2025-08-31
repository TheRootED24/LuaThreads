require "threads"
local thread = threads.thread;

function thread_hello(t)
	local t = thread.new(t);
	print("hello from Thread: "..t.id);
	local ret, dt = thread.attr.detach(t);
	print("Detached state: "..dt);

	thread.sleep(2,0); -- sleep for 2 seconds
	thread.complete(t);
	print(string.format("thread: %d has finished ...", t.id));
	thread.complete(t);
	thread.exit(t);
end

local function main(arg)
	local t0 = thread.new();
	local t1 = thread.new();
	local ret;

	-- set thread attr's to dethached
	ret = thread.attr.detach(t0, true);
	ret = thread.attr.detach(t1, true);

	print("\n----- Start Tests -----\n");
	print("Test 1: cancel thread early ..\n")

	ret = thread.create(t1, "thread_hello");
	thread.sleep(1,0)
	ret = thread.cancel(t1)  -- terminate thread t1 early

	print("\nTest 2: wait on thread to finish ..\n")

	ret = thread.create(t0, "thread_hello");

	while(threads.active() > 0) do
		thread.yield(); -- release cpu while we wait
	end

	print("\n----- Tests Complete -----\n");

	-- clean up thread attr's
	thread.attr.destroy(t0);
	thread.attr.destroy(t1);

end

main(arg)