/*****************************************************************************
lua50_profiler.c:
   Lua version dependent profiler interface
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clocks.h"
#include "core_profiler.h"

#include "lua.h"
#include "lauxlib.h"


/* called by Lua (via the callhook mechanism) */
static void callhook(lua_State *L, lua_Debug *ar) {
int currentline;
lua_Debug previous_ar;

   if (lua_getstack(L, 1, &previous_ar) == 0) {
      currentline = -1;
   } else {
      lua_getinfo(L, "l", &previous_ar);
      currentline = previous_ar.currentline;
   }
      

   lua_getinfo(L, "nS", ar);

   if (!ar->event) {
   	/* entering a function */
		lprofP_callhookIN((char *)ar->source, (char *)ar->name,
		                  (char *)ar->source, ar->linedefined,
		                  currentline);
	}
	else { /* ar->event == "return" */
		lprofP_callhookOUT();
	}
}


/* Lua function to exit politely the profiler                               */
/* redefines the lua exit() function to not break the log file integrity    */
/* The log file is assumed to be valid if the last entry has a stack level  */
/* of 1 (meaning that the function 'main' has been exited)                  */
static void exit_profiler(lua_State *L) {
	/* leave all functions under execution */
	while (lprofP_callhookOUT()) ;
	/* call the original Lua 'exit' function */
	lua_getglobal(L, "_exit");
	lua_call(L, 0, 0);
}


/* calculates the approximate time Lua takes to call a function */
static float calcCallTime(lua_State *L) {
clock_t timer;
char lua_code[] = "                                     \
                   function lprofT_mesure_function()    \
                   local i                              \
                                                        \
                      local t = function()              \
                      end                               \
                                                        \
                      i = 1                             \
                      while (i < 100000) do             \
                         t()                            \
                         i = i + 1                      \
                      end                               \
                   end                                  \
                                                        \
                   lprofT_mesure_function()             \
                   lprofT_mesure_function = nil         \
                 ";

   lprofC_start_timer(&timer);
   lua_dostring(L, lua_code);
   return lprofC_get_seconds(timer) / (float) 100000;
}


int luaopen_profiler(lua_State *L) {
float function_call_time;

	function_call_time = calcCallTime(L);

    lua_sethook(L, (lua_Hook)callhook, LUA_MASKCALL | LUA_MASKRET, 0);
    /* init with default file name and printing a header line */
    if (!lprofP_init_core_profiler(NULL, 1, function_call_time)) {
        printf("luaProfiler error: output file could not be opened!");
        exit(0);
    }

    /* use our own exit function instead */
    lua_getglobal(L, "exit");
    lua_setglobal(L, "_exit");
    lua_register(L, "exit", (lua_CFunction)exit_profiler);

    /* the following statement is to simulate how the execution stack is */
    /* supposed to be by the time the profiler is activated when loaded  */
    /* as a library.                                                     */

    /* for this to be true, your Lua 5.0 environment must be started in  */
    /* a simmilar way to this:                                           */
    /* lua -e 'local f, e1, e2 = loadlib("./luaprofiler_lua50.so", "init_profiler") assert(f, (e1 or "").."\t"..(e2 or "")) f()' <normal_lua_starting_point.lua>   */

    lprofP_callhookIN("", "(null)", "@init_profiler.lua", 0, -1);
	lprofP_callhookIN("", "luaopen_profiler", "(C)", -1, -1);

	return 1;
}


