require "threads"

local thread = threads.thread;
local mutex = thread.mutex.init(); -- initialize the mutex

local loops = arg[1] or 10;
local id = 0;

function print_cnt(cnt)
	io.write("cnt: "); io.write(cnt); io.write("\n")
end

function print_ms(m_id, m_state, s_id, s_state)
	io.write("thread id: "); io.write(m_id);
	io.write(" ");
	io.write("state: "); io.write(m_state)

	io.write("\nthread slave: "); io.write(s_id)
	io.write(" ");
	io.write("state: "); io.write(s_state);
	io.write("\n");
end

function print10(t, tt)
	local cnt = 0;
	local t = thread.new(t);
	local tt = thread.new(tt);
	thread.mutex.lock(mutex)
		print(string.format("Thread: %d Has lock %d", t.id, t.state))
		for i=1, loops do
			if(tt) then print_ms(t.id, t.state, tt.id, tt.state) end
			cnt = cnt + 1;
			print_cnt(cnt)
		end
	thread.mutex.unlock(mutex)
	thread.complete(t)
	return t
end

local function main(...)
	local t1 = thread.new()
	local t2 = thread.new()
	local t3 = thread.new()
	local t4 = thread.new()
	local t5 = thread.new()
	local t6 = thread.new()
	
	local t7 = thread.new()
	local t8 = thread.new()
	local t9 = thread.new()
	local t10 = thread.new()

	print("Start Threads")

	thread.create(t1, "print10", t2)
	thread.create(t2, "print10", t1)
	thread.create(t3, "print10", t4)
	thread.create(t4, "print10", t5)
	thread.create(t5, "print10", t6)
	thread.create(t6, "print10", t7)
	
	thread.create(t7, "print10", t8)
	thread.create(t8, "print10", t9)
	thread.create(t10, "print10",t9)
	



	thread.mutex.lock(mutex)
	print("Hello from main thread")
	thread.mutex.unlock(mutex);

	t1 = thread.join(t1)
	t2 = thread.join(t2)
	t3 = thread.join(t3)
	t4 = thread.join(t4)
	t5 = thread.join(t5)
	t6 = thread.join(t6)
	
	t7 = thread.join(t7)
	t8 = thread.join(t8)
	t9 = thread.join(t9)
	t10 = thread.join(t10)

	print(threads.active())
	


	print("All Threads Finished")

	print( t1, t2, t3, t4, t5, t6)
end

main(...)