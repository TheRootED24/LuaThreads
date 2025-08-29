require "threads"
local thread = threads.thread;

local loops = tonumber(arg[1]) or 10;
local cnt = 0;
local mutex = thread.mutex.init(); -- initialize the mutex

function print_cnt(cnt)
	io.write(cnt)
	io.write("\n")
end

function yield_test(t)
	local t = thread.new(t);

	thread.yield(t) -- wait for inital resume (optional)
	print("thread_start")
	while(thread.run(t)) do
		if(thread.state(t) ~= SUSPENDED ) then
			if(cnt < loops) then
				cnt = cnt + 1
			end
			print_cnt(cnt)
			thread.yield(t)
		end
		thread.yield() -- let the cpu tend to something else while we yield
	end
end

local function main(...)
	local t = thread.new()
	t = thread.create(t, "yield_test")
	print("start test")
	thread.sleep(1,0)
	while(cnt < loops) do
		thread.resume(t);
		--if(cnt < loops - 1000) then
		--	cnt = cnt + 1000
		--end
	end
	thread.join(t);
	print(string.format("Final: %s/%s", tostring(cnt) , tostring(arg[1] or 10)))
end

main(...)