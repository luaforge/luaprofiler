-- Util.lua
-- ========
-- Defines some functions util to many modules

local old_openfile = openfile
function openfile(fname, mode)
local hnd = old_openfile(fname, mode)
   if (hnd == nil) then
      error("openfile: Failed to open file '"..tostring(fname).."' with mode '"..tostring(mode).."'")
   else
      return hnd
   end
end

function anlU_cleanTable(table)
local ind
   ind = next(table, nil)
   while (ind) do
      table[ind]=nil
      ind = next(table, nil)
   end
end

function anlU_removeGraphFiles(log_file)
   dofile("Config.lua")
   anlC_graph_file_name = format(anlC_graph_file_name, log_file, "%s")
   i = 0
   while (readfrom(format(anlC_graph_file_name, i))) do
      readfrom()
      remove(format(anlC_graph_file_name, i))
      i = i + 1
   end
end
