local tab = require "tab"

function new_connection(t)
    if not(t) then t = tab.new() end
    local proxyTable = {t = t, int, char, string, bool, next = function(a) return(print(a)) end}

    local metaTable = {
        __index = function(tbl, key)

             if(key and tostring(key) == "int") then
                return(t:int())
            elseif(key and tostring(key) == "char") then
                return(t:char())
             elseif(key and tostring(key) == "string") then
                return(t:string())
             elseif(key and tostring(key) == "bool") then
                return(t:bool())
             end
        end,
        __newindex = function(tbl, key, value)
            if(key and tostring(key) == "int") then
                t:int(value)
            elseif(key and tostring(key) == "char") then
                t:char(value)
             elseif(key and tostring(key) == "string") then
                t:string(value)
             elseif(key and tostring(key) == "bool") then
                t:bool(value)
             end
        end,
        __call = function(tbl, key, value)
            print(key,value)
             if(key and tostring(key) == "int") then
                t:int(value)
            elseif(key and tostring(key) == "char") then
                t:char(value)
             elseif(key and tostring(key) == "string") then
                t:string(value)
             elseif(key and tostring(key) == "bool") then
                t:bool(value)
            elseif(key and tostring(key) == "next") then
                return("next")
             end
        end
    }
    setmetatable(proxyTable, metaTable)

    return proxyTable
end

function print_int(conn)
    print(conn.string, conn.int)
end

local c = new_connection()
local d = new_connection();


c.int = 1
c.string = "test"
c.bool = true
c.char = "A"

d.int = c.int
d.string = "dtest"
d.t = 0
--print("next", c.next)
c.int = 100
c.string = "hey you"
c("int",32)
--while(c:int())do end
c.new = 6
print_int(c);
print(c.string)
print(c.bool)
c.int = 10000
c.next("hello")
print("d = "..d.int);
print_int(d)
c.next("Great Scott!! it works")

