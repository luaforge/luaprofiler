-- string function
format = string.format
gsub = string.gsub
strsub = string.sub

-- io functions
local hnd_in
local hnd_out
openfile = io.open
read = function(hnd, pattern) if (pattern) then return hnd:read(pattern) else return hnd:read() end end
write = function (hnd, ...) if (type(hnd) ~= "string") then hnd:write(unpack(arg)) else hnd_out:write(hnd, unpack(arg)) end end
closefile = function(hnd) hnd:close() end
readfrom = function(fname) if (fname == nil) then hnd_in:close() else hnd_in = io.open(fname, "r") end return hnd_in end
writeto = function(fname) if (fname == nil) then hnd_out:close() else hnd_out = io.open(fname, "w") end return hnd_out end
seek = function(hnd, whence, offset) hnd:seek(whence, offset) end

-- standard api
dostring = function(code) return loadstring(code)() end

-- math functions
floor = math.floor

-- os functions
remove = function(fname) return os.remove(fname) end
