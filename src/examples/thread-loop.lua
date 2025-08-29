require "threads"

thread = threads.thread
mutex = thread.mutex.init()
local function sleep (a)
	local sec = tonumber(os.clock() + a)
	while (os.clock() < sec) do
	end
	return 0;
end


function test(t)
	thread.mutex.lock(mutex)
	local t = thread.new(t)
	print(string.format("thread: %d has locked the mutex\n", t.id))

	for i =1, arg[1] or 1000 do
		io.write(i)
		io.write("\nId: "); io.write(t.id);
		io.write("\n");
	end

	print(string.format("thread: %d has released the mutex\n", t.id));
	--threads.stats();
	thread.complete(t);
	thread.mutex.unlock(mutex);
	

end

function main(...)
	print("\n START THREAD TEST ...\n")
	local tidy = {}

	
	for x=1, 150 do
		thread.mutex.lock(mutex);
		tidy[x] = thread.new();
		--thread.sleep(0, (1 * #tidy)) -- 1 nano sec sleep x thread id
		thread.create(tidy[x], "test");
		thread.mutex.unlock(mutex);
		thread.sleep(0, (1 * #tidy));
	end
	
	thread.sleep(1,0); -- simulate work in main

	for i=1, #tidy do
		thread.join(tidy[i]);
		io.write("Joined thread: "); io.write(tidy[i].id); io.write("\n")
		tidy[i] = nil
	end

	print("\n END THREAD TEST ...\n")
end

main(...)