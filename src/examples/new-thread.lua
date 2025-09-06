local threads = require "threads"
local thread = threads.thread;

local mutex = thread.mutex.init();
gcnt = 0

function thread_function(nt)
    local t = thread.new(nt);
     local cnt = 0;
     local f = function()
        for i= 1, 10 do
            print("Thread id: "..t.id);
            print("PThread id: "..t.tid);
            cnt = cnt + 1;
        end
        return cnt
    end
    thread.mutex.lock(mutex);
    gcnt = gcnt + f();
    thread.mutex.unlock(mutex);
end

function thread_function1(nt)
    local t = thread.new(nt);
     local cnt = 0;
     local f = function()
        for i= 1, 20 do
            print("Thread id: "..t.id);
            print("PThread id: "..t.tid);
            cnt = cnt + 1;
        end
        return cnt
    end
    thread.mutex.lock(mutex);
    gcnt = gcnt + f();
    thread.mutex.unlock(mutex);
end

function thread_function2(nt)
    local t = thread.new(nt);
     local cnt = 0;
     local f = function()
        for i= 1, 30 do
            print("Thread id: "..t.id);
            print("PThread id: "..t.tid);
            cnt = cnt + 1;
            thread.sleep(0,9000000);
        end
        --thread.sleep(3,0);
        return cnt
    end
    thread.mutex.lock(mutex);
    gcnt = gcnt + f();
    thread.mutex.unlock(mutex);
end

local function main(...)
    local t1 = thread.new();
    local t2 = thread.new();
    local t3 = thread.new();

    local ret = thread.create(t1, "thread_function");
    local ret2 = thread.create(t2, "thread_function1");
    local ret3 = thread.create(t3, "thread_function2");
   thread.sleep(0,5);
    for i=1, 50 do
        thread.mutex.lock(mutex);
         gcnt = gcnt + 1;
         print(gcnt);
        thread.mutex.unlock(mutex);
    end
    ret = thread.join(t1);
    ret = thread.join(t2);
    ret = thread.join(t3);
    thread.mutex.lock(mutex);
    print("RET: "..ret);
    print(gcnt);
    thread.mutex.unlock(mutex);
end

main(...)