include config

COMMON_OBJS= src/clocks.o src/core_profiler.o src/function_meter.o src/stack.o
LUANG_OBJS= src/cgilua32_profiler.o
LUA32_OBJS= src/lua32_profiler.o
LUA40_OBJS= src/lua40_profiler.o
LUA50_OBJS= src/lua50_profiler.o


lua5: $(COMMON_OBJS) $(LUA50_OBJS)
	$(LD) -Bshareable -o $(LUA_50_OUTPUT) $(COMMON_OBJS) $(LUA50_OBJS)


clean:
	rm -f $(LUA_50_OUTPUT) src/*.o
