-- GraphToGDL.lua
-- ==============
-- Exports the graph represented in Lua to a graph represented in a language
-- called GDL (Graph Design Language), used by the aiSee graph viewer program

local luaValues2gmdValues = function (lua_values)
   for i, v in lua_values do
      write(i..': '..v..' ')
   end
end

local alocateColors = function ()
local values = {}
   values["colorentry 40"] = "225 225 225"
   values["colorentry 41"] = "200 200 200"
   values["colorentry 42"] = "175 175 175"
   values["colorentry 43"] = "150 150 150"
   values["colorentry 44"] = "125 125 125"
   values["colorentry 45"] = "100 100 100"
   values["colorentry 46"] = "75 75 75"
   values["colorentry 47"] = "50 50 50"
   values["colorentry 48"] = "25 25 25"
   values["colorentry 49"] = "0 0 0"
   luaValues2gmdValues(values)
end

local modules_ref = {}
local layout = {
   shapes = {"box", "trapezoid", "hexagon", "lparallelogram", "ellipse"},
   next_shape = 1}
function anlE_newModuleRef(module_name)
local shapes = layout.shapes
   module_name = gsub(module_name, "[\"'\\]", "_")
   modules_ref[module_name] = {title = module_name, shape = shapes[layout.next_shape]}
   layout.next_shape = layout.next_shape + 1
   if (layout.next_shape > 5) then layout.next_shape = 1 end
   return modules_ref[module_name]
end

function anlE_getModulesRef()
   return modules_ref
end


function anlE_openGraph(values)
   write('graph: {' ..
         'node.horizontal_order: -1\n')
   alocateColors()
end

function anlE_closeGraph()
   write('}')
end

function anlE_openDesignGraph(values)
   write('graph: {' ..
         'title: "design"' ..
         'label: "Design"' ..
         'node.info1: "' .. [[
Module node
===========

To open the detailed view of this module,
press u (unfold), then left-click and
right-click on the module's node to open
all its functions.
To close the detailed view, press f (fold)
then left-click and right-click in any
function of the module.
To close this window, right-click.
"]]..'\n')
end

function anlE_closeDesignGraph()
   write('}')
end

function anlE_openModule(module_name, values)
   module_name = gsub(module_name, "[\"'\\]", "_")
   local new_module_ref = anlE_newModuleRef(module_name)
   values = values or {}
   values["title"] = '"'..new_module_ref.title..'"'
   values["label"] = '"Module '..module_name..'"'
   values["node.shape"] = new_module_ref.shape
   values["state"] = 'folded'

   write('graph: {')
   luaValues2gmdValues(values)
end

function anlE_closeModule()
   write('}')
end

function anlE_openLegendGraph()
local values = {}
   values["title"] = '"legend"'
   values["label"] = '"Legend"'
   values["borderstyle"] = 'double'
   values["edge.linestyle"] = 'invisible'
   values["state"] = 'boxed' -- 'clustered'

   write('node.horizontal_order: 1' ..
         'graph: {')
   luaValues2gmdValues(values)
end

local last_legend_module = {ref = nil}
function anlE_newLegendModule(module_name)
   module_name = gsub(module_name, "[\"'\\]", "_")
   local layout_info = modules_ref[module_name]
   anlE_node{
      title = '"l'..layout_info.title..'"',
      label = '"Module '..module_name..'"',
      shape = layout_info.shape
   }

   if (last_legend_module.ref ~= nil) then
      anlE_rightnearedge{
         sourcename = '"l'..last_legend_module.ref.title..'"',
         targetname = '"l'..layout_info.title..'"'
      }
   end
   last_legend_module.ref = layout_info

   anlE_edge{
      sourcename = '"l'..layout_info.title..'"',
      targetname = '"'..layout_info.title..'"',
      linestyle  = 'invisible'
   }
end

function anlE_closeLegendGraph()
   anlE_node{
      title='"l0"',
      label='""',
      borderstyle="invisible"}

   anlE_rightbentnearedge{
      sourcename='"l0"',
      targetname='"l'..last_legend_module.ref.title..'"',
   }

   write('}')
end

function anlE_edge(values)
   write('edge: {')
   luaValues2gmdValues(values)
   write('}\n')
end

function anlE_rightnearedge(values)
   write('rightnearedge: {')
   luaValues2gmdValues(values)
   write('}\n')
end

function anlE_leftbentnearedge(values)
   write('leftbentnearedge: {')
   luaValues2gmdValues(values)
   write('}\n')
end

function anlE_rightbentnearedge(values)
   write('rightbentnearedge: {')
   luaValues2gmdValues(values)
   write('}\n')
end

function anlE_node(values)
   write('node: {')
   luaValues2gmdValues(values)
   write('}\n')
end

