-- LogGraphToDesignGraph.lua
-- =========================
-- Merges the vertices and edges of the log graph until we get the profiled
-- program's design graph

-- The log graph will be traversed via DFS and, for each vertex we will know
-- who are their successors

dofile("tDFS.lua")
dofile("Util.lua")
dofile("GraphToGDL.lua")

-- a function can be distinguished (almost fully) by the information present
-- in the log file on the fields "file_defined", "function_name", "line_defined"
local getUniqueId = function (log_vertex)
   return log_vertex.info.file_defined  .. "$" ..
          log_vertex.info.function_name .. "$" ..
          log_vertex.info.line_defined  .. "$"
end


local M_design_vertices = {}
local M_profile_info = {}

-- identifies the unique vertices
local getUniqueVertex = function (log_vertex)
local unique_id = getUniqueId(log_vertex)
   if (M_design_vertices[unique_id] == nil) then
      M_design_vertices[unique_id] = {}
      M_design_vertices[unique_id].file_defined  = log_vertex.info.file_defined
      M_design_vertices[unique_id].function_name = log_vertex.info.function_name
      M_design_vertices[unique_id].line_defined  = log_vertex.info.line_defined
      M_design_vertices[unique_id].local_time   = 0
      M_design_vertices[unique_id].total_time   = 0
      M_design_vertices[unique_id].called_times = 0
      M_design_vertices[unique_id].id = unique_id
      M_design_vertices[unique_id].successors = {}
   end
   return M_design_vertices[unique_id]
end


local M_vertices_by_module = {}

-- the vertices are also grouped by their modules to simplify the translation
-- into GDL (graph description language)
local groupByModule = function (vertex)
   vertex = getUniqueVertex(vertex)
   if (M_vertices_by_module[vertex.file_defined] == nil) then
      M_vertices_by_module[vertex.file_defined] = {}
   end
   M_vertices_by_module[vertex.file_defined][vertex] = 1
end


-- we can't forget to compute relevant informations
local computeEdgeInfo = function (edge, info)
local vertex = edge.vertex

   edge.steps = edge.steps + 1
   if (edge.steps > M_profile_info.max_steps) then
      M_profile_info.max_steps = edge.steps
   end

   edge.local_time_sum = edge.local_time_sum + info.local_time
   if (edge.local_time_sum > M_profile_info.max_local_time_sum) then
      M_profile_info.max_local_time_sum = edge.local_time_sum
   end

end

local computeVertexInfo = function (vertex)
local u_vertex = getUniqueVertex(vertex)

   M_profile_info.all_local_times_sum = M_profile_info.all_local_times_sum + vertex.info.local_time

   u_vertex.called_times = u_vertex.called_times + 1
   if (u_vertex.called_times > M_profile_info.max_called_times) then
      M_profile_info.max_called_times = u_vertex.called_times
   end

   u_vertex.local_time = u_vertex.local_time + vertex.info.local_time
   if (u_vertex.local_time > M_profile_info.max_local_time) then
      M_profile_info.max_local_time = u_vertex.local_time
   end

end


-- the profiled program's design don't have two repeated vertices (the
-- program don't have two repeated functions)
local connectAsSuccessor = function (u_ancestor, u_successor, successor_info)

   -- connect ancestor to successor
   if (u_ancestor.successors[u_successor] == nil) then
      u_ancestor.successors[u_successor] = {vertex = u_successor, steps = 0, local_time_sum = 0}
   end
   computeEdgeInfo(u_ancestor.successors[u_successor], successor_info)

end


local M_vertices_with_incomplete_successors_list = {}
local M_last_dfsn = {}

-- called whenever the dfsn becomes smaller: there will be no more successors
-- for the vertices with a greater dfsn, so they can be connected
local finalizeSuccesorsListForVerticesDeeperThan = function (dfsn)
local last_u_vertex
local u_vertex
local list = M_vertices_with_incomplete_successors_list

   u_vertex = getUniqueVertex(list[dfsn])
   for i = dfsn+1, list.last_n do
      last_u_vertex = u_vertex
      u_vertex = getUniqueVertex(list[i])
      connectAsSuccessor(last_u_vertex, u_vertex, list[i].info)
      list[i] = nil
   end
   list.last_n = dfsn
end


-- DFS preWORK: reconnect the vertices
local preWORK = function(vertex, dfsn)
local list = M_vertices_with_incomplete_successors_list
   groupByModule(vertex)
   computeVertexInfo(vertex)
   -- reconnect the vertices
   if (dfsn <= M_last_dfsn[1]) then
      finalizeSuccesorsListForVerticesDeeperThan(dfsn-1)
   end
   -- in DFS, if dfsn > last dfsn, dfsn = last dfsn + 1
   M_last_dfsn[1] = dfsn
   list[dfsn] = vertex
   list.last_n = list.last_n + 1
end


function anlD_DesignGraphToGDL()

   anlE_openGraph()

   anlE_openDesignGraph()
   for _file_defined, vertices in M_vertices_by_module do
      local file_defined = _file_defined
      -- strip leading '@' or '=' and the path and end up only with the file name
      do
         local c = strsub(file_defined, 1, 1)
         if ((c=='@') or (c=='=')) then
            file_defined = strsub(file_defined, 2)
         end
      end
      file_defined = gsub(file_defined, ".*/(.*)", "%1")

      anlE_openModule(file_defined)
      for vertex, _ in vertices do
         -- try to guess the function name
         if (vertex.function_name == "(null)") then
            if ((vertex.line_defined == 0) and
--                (vertex.current_line == -1) and
                (vertex.file_defined ~= "(C)")) then
               vertex.function_name = "main"
            else
               vertex.function_name = "(at line "..vertex.line_defined..")"
            end
         end
         anlE_node{
            title='"'..vertex.id..'"',
            label='"'..vertex.function_name..'"',
            bordercolor = floor((vertex.local_time/M_profile_info.max_local_time)*9)+40,
            borderwidth = floor((vertex.called_times/M_profile_info.max_called_times)*6)+1,
            info1='"' ..
                  format('total time (will be sum): %.2f\n',
                         vertex.total_time) ..
                  format('local time:   %.2f (%.2f%%)\n',
                         vertex.local_time, (vertex.local_time/M_profile_info.all_local_times_sum)*100) ..
                  format('called times: %d (%.2f%%)\n',
                         vertex.called_times, (vertex.called_times/M_profile_info.max_called_times)*100) ..
                  format('average total time (will be sum): %.2f\n',
                         vertex.total_time/vertex.called_times) ..
                  format('average local time:   %.6f',
                         vertex.local_time/vertex.called_times) ..
                  '"'}
      end
      anlE_closeModule()
   end

   anlE_closeDesignGraph()

   anlE_openLegendGraph()
   local mr = anlE_getModulesRef()
   for file_defined, layout_info in mr do
      anlE_newLegendModule(file_defined)
   end
   anlE_closeLegendGraph()

   for i, v in M_design_vertices do
      v.successors = v.successors or {}
      for i2, v2 in v.successors do
         anlE_edge{
            sourcename = '"'..v.id..'"',
            targetname = '"'..v2.vertex.id..'"',
            thickness = floor((v2.steps/M_profile_info.max_called_times)*6)+1,
            color = floor((v2.local_time_sum/M_profile_info.max_local_time)*9)+40
         }
--         print("/*")
--         print("v2.steps=",v2.steps)
--         print("%M_profile_info.max_steps=",%M_profile_info.max_steps)
--         print("v2.local_time_sum=",v2.local_time_sum)
--         print("%M_profile_info.max_local_time_sum=",%M_profile_info.max_local_time_sum)
--         print("*/")
--         print("caminho "..v.id.."->"..v2.vertex.id.." foi percorrido "..v2.steps.." vezes com o tempo "..(v2.local_time_sum/%M_profile_info.all_local_times_sum)*100)
      end
   end

   anlE_edge{
      sourcename = '"legend"',
      targetname = '"design"',
      linestyle  = "invisible",
   }

   anlE_closeGraph()

end


function anlD_getDesignGraph(log_file)

   anlU_cleanTable(M_vertices_with_incomplete_successors_list)
   M_vertices_with_incomplete_successors_list.last_n = -1

   anlU_cleanTable(M_design_vertices)

   anlU_cleanTable(M_vertices_by_module)

   anlU_cleanTable(M_profile_info)
   M_profile_info.all_local_times_sum = 0
   M_profile_info.max_steps           = 0
   M_profile_info.max_local_time_sum  = 0
   M_profile_info.max_called_times    = 0
   M_profile_info.max_local_time      = 0

   M_last_dfsn[1] = -1

   anlT_DFS(log_file, preWORK)
   finalizeSuccesorsListForVerticesDeeperThan(0)

   anlD_DesignGraphToGDL()
   if (1) then return end

   print("**********************")
   for i, v in M_design_vertices do
      print("vertice "..i.." tem "..tostring(v.s).." sucessores e "..tostring(v.a).." antecessores")
   end

   print("**********************")
   for i, v in M_design_vertices do
      v.successors = v.successors or {}
      for i2, v2 in v.successors do
         print("caminho "..v.id.."->"..v2.vertex.id.." foi percorrido "..v2.steps.." vezes com o tempo "..(v2.local_time_sum/M_profile_info.all_local_times_sum)*100)
      end
   end
end
