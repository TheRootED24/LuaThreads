require "threads"
local thread = threads.thread;

function thread_hello(t)
	local t = thread.new(t);
	print("hello from Thread: "..t:id());
	local dt = thread.attr.scope(t);
	print("Thread Scope: "..dt);

	--thread.sleep(2,0); -- sleep for 2 seconds
	print(string.format("thread: %d has finished ...", t:id()));

	thread.complete(t);
end

local function main(arg)
	local ret, sched, policy;
	local t0 = thread.new();

	-- set thread attr's to dethached
	ret = thread.attr.sched_policy(t0, "fifo")
	ret = thread.attr.sched_param(t0, 30); -- set thread t0->attr detached state
	ret, sched = thread.attr.sched_param(t0); -- set thread t0->attr detached state
	ret, policy = thread.attr.sched_policy(t0);
	print("Sched Policy:", policy, ret);
	print("Sched Param :	", sched, ret);
	print("\n----- Start Tests -----\n");

	ret = thread.create(t0, "thread_hello");
	ret = thread.join(t0); -- wait on thread t1 then free resources

	print("\n----- Tests Complete -----\n");
end

main(arg)