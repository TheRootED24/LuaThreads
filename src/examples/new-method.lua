require "threads"
local thread = threads.thread;
local mutex = thread.mutex.init();
function print_tid(t)
	local t = thread.new(t);
	print(t.id)
	while(t.run)do
		while(t.state == WORKING ) do
			thread.mutex.lock(mutex);
			print(string.format("Thread TID: %d\tThread ID: %d\tThread STATE: %d\n", t.tid, t.id, t.state));
			thread.mutex.unlock(mutex);
			--thread.yield(t);
		end
		print(t.state)
		--thread.resume(t);
	end

	--thread.complete(t);

end


local function main(...)
	local t1 = thread.new();
	local t2 = thread.new();

	t1 = thread.create(t1,"print_tid");
	t2 = thread.create(t2,"print_tid");
	--print(thread.state(t1))
	--for i=1, 25 do
	--thread.mutex.lock(mutex);	
	threads.stats();
	thread.yield(t1)
	thread.yield(t2)
	thread.sleep(1,0);
	thread.resume(t1);
	thread.resume(t2);
	thread.sleep(3,0);
	--thread.mutex.unlock(mutex);
		thread.complete(t1)
		thread.complete(t2)
		
	thread.join(t1);
	thread.join(t2);

	print("complete")
end

main(...)