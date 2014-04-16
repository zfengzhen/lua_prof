lua_prof
========

### usage
for lua 5.2 profiling  

### notice
should compile lua 5.2 with [-fPIC]  

### test
```lua
local lua_prof = require("lua_prof")
                                                                                                                                  
local function is_prime(i)
    k = math.sqrt(i)
    for j = 2, k do
        if i%j == 0 then
            return false
        else
            j = j + 1
        end
    end
    return true
end

local function all_prime(max)
    local tb = {}
    for i = 1, max do
        if is_prime(i) then
            table.insert(tb, i)
        end
    end
    return tb
end
  
lua_prof.init()
local x = all_prime(100000) 
local prof_ret = lua_prof.fini()
                               
for _, v in ipairs(prof_ret) do
    print(v)
end
```

### result  

```
function: all_prime
[Lua], [local  ], times [1], total [369ms], avg [369228us]
function: is_prime
[Lua], [upvalue], times [100000], total [286ms], avg [2us]
function: sqrt
[C  ], [field  ], times [100000], total [53ms], avg [0us]
function: insert
[C  ], [field  ], times [9593], total [7ms], avg [0us]
function: fini
[C  ], [field  ], times [1], total [0ms], avg [0us]
```


