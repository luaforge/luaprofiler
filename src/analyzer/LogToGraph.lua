-- LogToGraph.lua
-- ==============
-- Converts the profiler log file into an easily traversable graph

-- The problem of converting the log file into a graph corresponds to the
-- problem of turning a "reversed" depth-first search (with the dfsn, the
-- depth-first search number for each vertex) back into the corresponding
-- graph (which is a path on the profiled program's design graph)

-- "reversed" depth-first search means that instead of being "printed" before
-- visiting the vertex's descendents (as done in a normal depth-first search),
-- the vertex is "printed" only after all his descendents have been processed
-- (in a depth-first search recursive algorithm).

-- Properties of the graph and the log file with interest to the conversion:
-- 1) Each vertex points to 0 or more successors
-- 2) Each vertex has 1 ancestor, except the starting vertex, with dfsn = 0
-- 3) Let v be a vertex and s one of its successors: dfsn(s) = dfsn(v) + 1
-- 4) Each vertex knows of the number of successors he owns
-- 5) The graph is represented as follows: the vertices are grouped by their
--    dfsn in lists. This lists are filled by adding log entries in the order
--    they appear
-- 6) So, the graph can be traversed if this rules are followed:
--    - the traversal starts at the starting vertex, dfsn = 0
--    - let v be a vertex with dfsn = n, his successors will be the vertices in
--      the next list (of dfsn = n + 1) at the positions between the sum of the
--      "number of successors" of the vertices that appear before v (in the v
--      list) + 1 and this position plus the "number of successors" of v.

-- Other properties of the graph:
-- 7) From 2, there is only one path from one vertex to another
-- 8) From 2, the graph is acyclic
-- 9) Let v be a vertex and w any vertex reachable from v; from 3 and 7:
--    dfsn(v) < dfsn(w)


dofile("LogIO.lua")

local store = function (vertex, dfsn, number_of_successors)
local hnd

   hnd = openfile(format(anlC_graph_file_name, dfsn), "a")

   write(hnd,
      "return {info={"  ,
      "stack="          , vertex.stack         , ",",
      "file_defined='"  , vertex.file_defined  , "',",
      "function_name='" , vertex.function_name , "',",
      "line_defined="   , vertex.line_defined  , ",",
      "current_line="   , vertex.current_line  , ",",
      "local_time="     , vertex.local_time    , ",",
      "total_time="     , vertex.total_time    , ",",
      "log_line="       , vertex.log_line      , "},",

      "number_of_successors=", number_of_successors, "}",
      "\n"
   )

   closefile(hnd)

end


local depthFirstListing_to_Graph = function ()
local dfsn
local vertex
local number_of_successors

   number_of_successors = {}

   vertex = anlL_getNextEntry()
   while (vertex) do

      dfsn = vertex.stack

      if (number_of_successors[dfsn-1] == nil) then
         number_of_successors[dfsn-1] = 1
      else
         number_of_successors[dfsn-1] = number_of_successors[dfsn-1] + 1
      end

      store(vertex, dfsn, number_of_successors[dfsn] or 0)
      number_of_successors[dfsn] = nil

      vertex = anlL_getNextEntry()

   end

end

function anlG_convert(log_file)
   dofile("Config.lua")
   anlC_graph_file_name = format(anlC_graph_file_name, log_file, "%s")

   anlL_openLog(log_file)
   depthFirstListing_to_Graph()
   anlL_closeLog()
end


