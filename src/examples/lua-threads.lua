require "threads"
local thread = threads.thread;

function new_thread(t)
    if not(t) then t = thread.new() end
    local proxyTable = {thread = t, run , id, tid, state, mutex, sem}

    local metaTable = {
        __index = function(tbl, key)

             if(key and tostring(key) == "id") then
                return(t:id())
	     elseif(key and tostring(key) == "tid") then
                return(t:tid())
            elseif(key and tostring(key) == "state") then
                return(t:state())
             elseif(key and tostring(key) == "run") then
                return(t:run())
	     elseif(key and tostring(key) == "mutex") then
                return(t:mutex())
             elseif(key and tostring(key) == "sem") then
                return(t:sem())
             end
        end,
        __newindex = function(tbl, key, value)
            if(key and tostring(key) == "run") then
                t:run(value)
             elseif(key and tostring(key) == "mutex") then
                t:mutex(value)
             elseif(key and tostring(key) == "sem") then
                t:sem(value)
             end
        end,
    }
    setmetatable(proxyTable, metaTable)

    return proxyTable
end

local t = new_thread();

print(t.id)
print(t.state)
print(t.run)
t.state = 10;
thread.complete(t.thread)

print(t.state)