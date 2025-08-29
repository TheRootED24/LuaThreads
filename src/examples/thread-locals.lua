require "threads"
local thread = threads.thread;

local cnt = 0
local res = 0
local mutex = thread.mutex.init(); -- initialize the mutex

function get_cnt()
	return cnt;
end

function set_cnt(t, l_cnt)
	thread.mutex.lock(t)
	cnt = l_cnt
	thread.mutex.unlock(t)
end

function local_test(t)
	local cnt = 0
	local t = thread.new(t);
	
	while(thread.run(t)) do
		if(thread.state(t) ~= SUSPENDED ) then
			--thread.yield(t)
			for i = 1, arg[1] or 10 do
				thread.mutex.lock(mutex)
				cnt = cnt + 1
				thread.mutex.unlock(mutex)
				io.write("local cnt: ")
				io.write(cnt)
				io.write("\n")
			end
			break;
		end
	end
	res = cnt
end

local function main(...)
	local t = thread.new()
	t = thread.create(t, "local_test")

	for i=1, arg[1] or 10 do
		--sleep(arg[2] or 0.01) -- add a minmimal delay to prevent thread deadlock
		thread.sleep(0,100000000)
		io.write("global cnt: ")
		io.write(cnt)
		io.write("\n")
		thread.mutex.lock(mutex);
		cnt = cnt - 1
		thread.mutex.unlock(mutex);
		--thread.resume(t);
	end
	thread.join(t, true);
	--thread.complete(t);
	print(string.format("Global Final: %d", cnt))
	print("local cnt --> global cnt")
	print(string.format("Updated Global Final: %d", res))
	
end


main(...)