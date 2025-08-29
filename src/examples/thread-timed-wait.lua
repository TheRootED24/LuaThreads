require "threads"
local thread = threads.thread;

local mutex = thread.mutex.init();
local wait_cond = thread.cond.new();
local data_ready = 0;

local wait = tonumber(arg[1]) or 0;
local trys = tonumber(arg[2]) or 0;

local ETIMEDOUT = 110

function producer(t)
	local t = thread.new(t);
	print("Producer: Producing data...");
	thread.sleep((wait + 3), 0);	-- Sleep to simulate proccesing data
	thread.mutex.lock(mutex);
	data_ready = 1;
	print("Producer: Data is ready. Signaling consumer.");
	thread.cond.signal(wait_cond); -- Signal the condition variable
	thread.mutex.unlock(mutex);

	thread.complet(t);
	return;
end

function consumer(ct, pt)
	local ct = thread.new(ct);
	local pt = thread.new(pt)
	thread.mutex.lock(mutex);
	local ret = 0;
	local ts = {wait, 0}
	while(data_ready ~= 1 and trys > 0 ) do
		ret = thread.cond.timedwait(wait_cond, mutex, ts);
		if(ret == ETIMEDOUT ) then
			print("Consumer: Timed out waiting for data.");
			trys = trys - 1;
		elseif (ret ~= 0) then
			print("thread.cond.timedwait error");
			thread.cancel(pt)
			return;
		end
	end
	if(data_ready == 1) then
		print("Consumer: Data received!");

	else
		print("Consumer: Too slow sucker joe!");
	end
	thread.mutex.unlock(mutex);
	thread.complet(t);
	return;
end

local function main(...)
	local pt = thread.new();
	local ct = thread.new();
	--if(arg[2]) then trys = tonumber(arg[2]) end;

	thread.create(pt, "producer");
	thread.create(ct, "consumer", pt);

	thread.join(pt);
	thread.join(ct);

	thread.mutex.destroy(mutex);
	thread.cond.destroy(wait_cond);

	return 0;
end

main(...)