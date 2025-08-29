require "threads"
local thread = threads.thread;
local mutex = thread.mutex.init();

function print_tid( t, pt )
   local t = thread.new(t);
   local pt = thread.new(pt);

   while( thread.state(t) == WORKING) do -- or t:status()
    thread.mutex.lock(mutex);
      print("thread_num",t:id(), "thread_id",t:tid())
      print("thread_num", pt:id(), "thread_id", pt:tid())
    thread.mutex.unlock(mutex);
    thread.yield(t) -- or t:yield()
   end
end

local function main(...)
   local t1 = thread.new();
   local t2 = thread.new();

   thread.create(t1, "print_tid",t2);
   thread.create(t2, "print_tid",t1);

   for i=1, 10 do
      thread.mutex.lock(mutex);
        thread.resume(t1) -- or t1:resume()
      thread.mutex.unlock(mutex);
   end

   t1 = thread.join(t1);
   print("ret:",t1)
   t2 = thread.join(t2);
   print("ret:",t2)
end

main(...)