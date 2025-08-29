require "threads"
local thread = threads.thread;

local mutex = thread.mutex.init();

local p_cond = thread.cond.new();
local c_cond = thread.cond.new();

local buf_size = 5;
local buf = {0};

local run = true;
local shutdown = tonumber(arg[1]) or 10;
local waiting = false;

-- thread safe print functions --
local function print_produced(id, item, index)
	io.write("PRODUCER THREAD ID: "); io.write(id);
	io.write(" ");
	io.write("PRODUCED ITEM: "); io.write(item);
	io.write(" ");
	io.write("AT INDEX: "); io.write(index);
	io.write("\n");
end

local function print_consumed(id, item , index)
	io.write("CONSUMER THREAD ID: "); io.write(id);
	io.write(" ");
	io.write("CONSUMED ITEM: "); io.write(item);
	io.write(" ");
	io.write("AT INDEX: "); io.write(index);
	io.write("\n");
end

local function print_full(caller, id)
	io.write(caller); io.write(id);
	io.write(" ");
	io.write(": Buffer full, waiting...\n");
end

local function print_empty(caller, id)
	io.write(caller); io.write(id);
	io.write(" ");
	io.write(": Buffer empty, waiting...\n");
end

-- thread functions must be global !!
function producer(t)
	local t = thread.new(t)
	while(threads.active() > 1 ) do -- producer is the LAST to close
		thread.mutex.lock(mutex);
		while(#buf == buf_size ) do
			print_full("Producer", t.id or nil);
			-- lua print functions are not thread safe !!!
			--print("Producer: 0 Buffer full, waiting...");
			if(threads.active() > 1 and shutdown > threads.active() + 1) then
				thread.cond.wait(p_cond, mutex);
			else
				break;
			end
		end

		if(#buf < buf_size) then
			buf[#buf+1] = shutdown;
		else
			buf[1] = shutdown;
		end
		-- lua print functions are not thread safe !!!
		--print(string.format("Producer: %d Produced %d at index %d\n", t.id, buf[#buf] or "test", #buf));
		print_produced(t.id, buf[#buf], #buf)
		thread.cond.broadcast(c_cond, mutex);

		if(shutdown > 0) then
			shutdown = shutdown - 1;
		end
		thread.mutex.unlock(mutex);

		if(threads.active() < 2) then
			thread.cond.broadcast(c_cond, mutex); -- signal any waiting consumers
			break;
		end
	end

	print("Producer: 0 wait for consumers to finish ...")
	while(threads.active() > 1) do
		thread.yield(); -- release cpu 
	end

	print(string.format("Producer: %d Shutting Down ...", t.id));
	thread.complete(t); -- thats all folks
end

function consumer(t, pt)
	local t = thread.new(t);
	local pt = thread.new(pt); -- access to the producer thread
	--print(string.format("PRODUCER THREAD ID: %d\n", pt.id))
	while(shutdown > 1) do
		thread.mutex.lock(mutex);
		while(#buf == 0 and threads.active() > 1) do
			print_empty("Consumer: ", t.id or nil)
			-- lua print functions are not thread safe !!!
			--print(string.format("Consumer: %d : buffer empty", t.id));
			if (threads.active() > 0 ) then
				thread.cond.wait(c_cond, mutex);
			else
				break;
			end
		end
		-- lua print functions are not thread safe !!!
		--print(string.format("Consumer %d: Consumed %d from index %d\n", t.id, buf[#buf], #buf));
		print_consumed(t.id, buf[#buf], #buf)
		buf[#buf] = nil;
		thread.cond.signal(p_cond, mutex);
		thread.mutex.unlock(mutex);

	end
	print(string.format("Consumer: %d Shutting Down", t.id));
	thread.complete(t); -- all done, thank you for you service
end

local function main(arg)
	local t1 = thread.new();
	local t2 = thread.new();
	local t3 = thread.new();
	local t4 = thread.new();
	local t5 = thread.new();
	local t6 = thread.new();
	local t7 = thread.new();
	local t8 = thread.new();
	--local t9 = thread.new();
	--local t10 = thread.new();



	-- producer
	local ret = thread.create(t1, "producer");

	-- consumers
	ret = thread.create(t2, "consumer");
	ret = thread.create(t3, "consumer");
	ret = thread.create(t4, "consumer");
	ret = thread.create(t5, "consumer");
	ret = thread.create(t6, "consumer");
	ret = thread.create(t7, "consumer");
	ret = thread.create(t8, "consumer");
	--ret = thread.create(t9, "consumer");
	--ret = thread.create(t10, "consumer");



	-- join the consumsers first
	ret = thread.join(t2);
	ret = thread.join(t3);
	ret = thread.join(t4);
	ret = thread.join(t5);
	ret = thread.join(t6);
	ret = thread.join(t7);
	ret = thread.join(t8);
	--ret = thread.join(t9);
	--ret = thread.join(t10);



	-- join the producer
	ret = thread.join(t1);

	-- show the threads and there states
	threads.stats();

	-- clean up
	ret = thread.cond.destroy(p_cond);
	ret = thread.cond.destroy(p_cond);
	ret = thread.mutex.destroy(mutex);
	
	print("complete");

end

main(arg);