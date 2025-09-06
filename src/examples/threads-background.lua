local threads = require "threads"
local thread = threads.thread

-- main thread lock 
local lock = thread.mutex.init();

local results = { 0, 0 }

-- some thread objects
local t1 = thread.new()
local t2 = thread.new()

-- function for threads to run
function test(nt)
	--thread.mutex.lock(lock)
    
	local t = thread.new(nt)
    --threads.stats();
	print(string.format("\nthread: %d has started\n", t.id))
    
	for i =1, (arg[1] * (200 + t.id)) or 100 do
		results[t.id] = i;
	end

	thread.complete(nt)
	print(string.format("\nthread: %d has completed\n", t.id))
	threads.stats();
	--thread.mutex.unlock(lock)
    --return OK
end

-- main function
local function main (...)
print("\n START THREAD TEST ...\n")
thread.create(t1, "test")
thread.create(t2, "test")
--t2:create("test")
--thread.sleep(0,1)
for i=1, 1000 do
    print("main: "..i);
    if(threads.active() > 0) then
        print(results[1]*results[2])
            break;
    end
end

io.write("\nwaiting on threads to finish ...\n")
while(threads.active() > 1 ) do
    thread.yield()
end

t1 = thread.join(t1);
t2 = thread.join(t2);
--t2 = t2:join()

print("all threads completed")
print("\n END THREAD TEST ...\n")

--for k,v in pairs(results) do
  --  print(v);
--end

print("final cnt: ".. (results[1]*results[2]))
end

-- i dunno, call main ..
main (...)