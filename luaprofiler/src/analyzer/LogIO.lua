-- LogIO.lua
-- =========
-- Parses a profiler log file entries


local M_log_format = "([^|]*)|([^|]*)|([^|]*)|([^|]*)|([^|]*)|([^|]*)|([^|]*)"
local M_file_hnd = {}
local M_current_line = {}

function anlL_openLog(file)
   M_file_hnd[1] = openfile(file, "r")
   -- field names line (ignore)
   read(M_file_hnd[1])
   M_current_line[1] = 2
end


function anlL_closeLog()
   closefile(M_file_hnd[1])
end


-- reads a log file line and returns as a table (parsed)
function anlL_getNextEntry()
local line
local entry = {}
local log_line = M_current_line[1]

   -- stack empty: read
   line =  read(M_file_hnd[1])
   if (line == nil) then
      return nil
   end

   gsub(line, M_log_format, function (stack, file_defined, function_name,
                                       line_defined, current_line, local_time, total_time)
      entry.stack         = tonumber(stack)
      entry.file_defined  = file_defined
      entry.function_name = function_name
      entry.line_defined  = line_defined
      entry.current_line  = current_line
      entry.local_time    = local_time
      entry.total_time    = total_time
      entry.log_line      = log_line
   end)
   M_current_line[1] = M_current_line[1] + 1
   return entry
end


