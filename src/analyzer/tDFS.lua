-- tDFS.lua
-- ========
-- Performs a depth-first search traversal on a graph and do a 'preWORK' -
-- executed at the first moment each vertex is visited

dofile("Util.lua")


local M_stack_info = {}

local getNextVertex = function (dfsn)
local line

   if (M_stack_info[dfsn] == nil) then
      M_stack_info[dfsn] = {next_read_pos = 0}
   end

   M_stack_info[dfsn].hnd = openfile(format(anlC_graph_file_name, dfsn), "r")
   seek(M_stack_info[dfsn].hnd, "set", M_stack_info[dfsn].next_read_pos)
   line = read(M_stack_info[dfsn].hnd)
   M_stack_info[dfsn].next_read_pos = seek(M_stack_info[dfsn].hnd)

   if ((line == nil) or (line == "")) then
      error("Inconsistent log file at stack "..(dfsn or 'nil'))
   end

   closefile(M_stack_info[dfsn].hnd)

   return dostring(line)

end


-- keep track of all bifurcations
-- keep visiting vertices until a vertex has no successors
-- return to the last bifurcation with unvisited vertices and restart
local dfs = function (preWORK)
local dfsn
local vertex
local vertices_with_unvisited_successors
local vwus_last
local last_bifurcation

   vertices_with_unvisited_successors = {}
   vwus_last = 0

   dfsn = 0
   repeat

      vertex = getNextVertex(dfsn)
      preWORK(vertex, dfsn)

      -- no successors to follow
      if (vertex.number_of_successors == 0) then
         last_bifurcation = vertices_with_unvisited_successors[vwus_last]

         if (last_bifurcation == nil) then
            break
         end

         -- proceed from the last bifurcation
         dfsn = last_bifurcation.info.stack
         last_bifurcation.number_of_successors = last_bifurcation.number_of_successors - 1
         if (last_bifurcation.number_of_successors == 0) then
            tremove(vertices_with_unvisited_successors, vwus_last)
            vwus_last = vwus_last - 1
         end
      -- found a bifurcation
      elseif (vertex.number_of_successors > 1) then
         vertex.number_of_successors = vertex.number_of_successors - 1
         tinsert(vertices_with_unvisited_successors, vertex)
         vwus_last = vwus_last + 1
      end

      dfsn = dfsn + 1

   until (nil)

end


function anlT_DFS(log_file, preWORK)
   dofile("Config.lua")
   anlC_graph_file_name = format(anlC_graph_file_name, log_file, "%s")

   anlU_cleanTable(M_stack_info)

   dfs(preWORK)
end


