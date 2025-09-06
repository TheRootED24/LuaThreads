local threads = require "threads"
local thread = threads.thread

-- main thread lock 
local lock = thread.mutex.init() 

-- some thread objects
local t1 = thread.new()
local t2 = thread.new()

-- function for threads to run
function test(nt)
	thread.mutex.lock(lock)
	local t = thread.new(nt)
	print(string.format("\nthread: %d has locked the mutex\n", t.id))

	for i =1, arg[1] or 10 do
		io.write(i) 
		io.write("\nId: "); io.write(t.id);
		io.write("\n");
	end

	--thread.complete(t)
	thread.error(t)
	print(string.format("\nthread: %d has released the mutex\n", t.id))
	threads.stats();
	thread.mutex.unlock(lock)
	return 1;
end

-- main function
local function main (...)
print("\n START THREAD TEST ...\n")
thread.create(t1, "test")
thread.create(t2, "test")
--t2:create("test")

thread.mutex.lock(lock)
io.write("thread: main has the locked the mutex\n")
for i =1, arg[1] or 10 do
	io.write(i)
	io.write("\nId: main\n")
end
print("thread: main released the mutex\n")
thread.mutex.unlock(lock)

t1 = thread.join(t1);
t2 = thread.join(t2);
--t2 = t2:join()

print("all threads completed")
print("\n END THREAD TEST ...\n")
threads.stats();

end

-- i dunno, call main ..
main (...)