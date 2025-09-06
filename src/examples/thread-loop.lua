local threads = require "threads"
local thread = threads.thread

local mutex = thread.mutex.init()
local queue = thread.buf.new();
queue.cnt = 0;

local gcnt = 0 

local tnum = arg[1] or 10;
local loops = arg[2] or 10;

function test(nt)
	--thread.mutex.lock(mutex)
	local cnt = 0;
	
	--print(string.format("thread: %d has locked the mutex\n", t.id))
	local function f()
		--local t = thread.new(n)
		--io.write("\nthread: "); io.write(t.id); io.write(" has locked the mutex\n");
		for i =1, loops do
		--	thread.mutex.lock(mutex);
			--print(string.format("thread: %d has locked the mutex\n", t.id))
			--io.write(i)
		--	io.write("\nId: "); io.write(t.id);
		--	io.write("\nCnt: "); io.write(cnt);
			cnt = queue.cnt;
			queue.cnt =  cnt + 1;
			--io.write("\n");
			--thread.mutex.unlock(mutex);
		end
		--thread.complete(t);
		--print(string.format("thread: %d has released the mutex\n", t.id));
		--io.write("\nthread: "); io.write(t.id); io.write(" has released the mutex\n");
		return cnt
	end
	
	thread.mutex.lock(mutex)
	local t = thread.new(nt)
	io.write("\nId: "); io.write(t.id);
	io.write("\n Queue: "); io.write(queue.cnt)
	cnt = f();
	io.write("\n Updated Queue: "); io.write(queue.cnt)
	thread.complete(t);
	thread.mutex.unlock(mutex);
	--return OK;
	--threads.stats();
end

function main(...)
	print("\n START THREAD TEST ...\n")
	local tidy = {}
	
 	for x=1, tnum do
		thread.mutex.lock(mutex);
		tidy[x] = thread.new();
		thread.create(tidy[x], "test");
		--print("cnt "..gcnt)
		io.write("\n Queue: "); io.write(queue.cnt)
		thread.mutex.unlock(mutex);
		thread.sleep(0, (1* #tidy));
		
	end
	
	thread.sleep(1,0); -- simulate work in main

	for i=1, #tidy do
		--thread.mutex.lock(mutex);
		print(queue.cnt)
		thread.join(tidy[i]);
		thread.mutex.lock(mutex);
		io.write("\nJoined thread: "); io.write(tidy[i].id); io.write("\n")
		thread.mutex.unlock(mutex);
		tidy[i] = nil
		--thread.mutex.unlock(mutex);
	end
	threads.stats();
	gcnt = queue.cnt
	print(string.format("final cnt: %d/%d", queue.cnt, (tnum*loops or 100)))
	print("\n END THREAD TEST ...\n")
	print(queue.cnt);
	queue = nil
end

main(...)