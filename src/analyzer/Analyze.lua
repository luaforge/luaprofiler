dofile("LogToGraph.lua")
dofile("LogGraphToDesignGraph.lua")


if ((profile_data == nil) or (gdl_output == nil)) then
print('Usage: lua profile_data="<file>" gdl_output="<file>" Analyze.lua')
exit()
end

print("converting...")
anlG_convert(profile_data)
print("writting GDL...")
writeto(gdl_output)
anlD_getDesignGraph(profile_data)
writeto()
print("cleaning...")

dofile("Config.lua")
anlC_graph_file_name = format(anlC_graph_file_name, profile_data, "%s")
i = 0
while (readfrom(format(anlC_graph_file_name, i))) do
   readfrom()
   remove(format(anlC_graph_file_name, i))
   i = i + 1
end

