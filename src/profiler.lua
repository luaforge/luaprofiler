local f, e1, e2 = loadlib("./luaprofiler_lua50.so", "luaopen_profiler")
assert(f, (e1 or "").."\t"..(e2 or ""))
f()

local function pack(...)
  return arg
end

local resume = coroutine.resume

function coroutine.resume(...)
  profiler.pause()
  local res = pack(resume(unpack(arg)))
  profiler.resume()
  return unpack(res)
end

local yield = coroutine.yield

function coroutine.yield(...)
  profiler.pause()
  local res = pack(yield(unpack(arg)))
  profiler.resume()
  return unpack(res)
end
