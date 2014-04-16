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
