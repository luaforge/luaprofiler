/*****************************************************************************
lua50_profiler.c:
   Lua version dependent profiler interface
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clocks.h"
#include "core_profiler.h"
#include "function_meter.h"

#include "lua.h"
#include "lauxlib.h"
#include "compat-5.1.h"

/* Indices for the main profiler stack and for the original exit function */
static int exit_id;

/* called by Lua (via the callhook mechanism) */
static void callhook(lua_State *L, lua_Debug *ar) {
   int currentline;
   lua_Debug previous_ar;
   lprofP_STATE* S;
   lua_pushlightuserdata(L, L);
   lua_gettable(L, LUA_REGISTRYINDEX);
   S = (lprofP_STATE*)lua_touserdata(L, -1);

   if (lua_getstack(L, 1, &previous_ar) == 0) {
      currentline = -1;
   } else {
      lua_getinfo(L, "l", &previous_ar);
      currentline = previous_ar.currentline;
   }
      
   lua_getinfo(L, "nS", ar);

   if (!ar->event) {
   	/* entering a function */
		lprofP_callhookIN(S, (char *)ar->source, (char *)ar->name,
		                  (char *)ar->source, ar->linedefined,
		                  currentline);
	}
	else { /* ar->event == "return" */
		lprofP_callhookOUT(S);
	}
}


/* Lua function to exit politely the profiler                               */
/* redefines the lua exit() function to not break the log file integrity    */
/* The log file is assumed to be valid if the last entry has a stack level  */
/* of 1 (meaning that the function 'main' has been exited)                  */
static void exit_profiler(lua_State *L) {
	lprofP_STATE* S;
	lua_pushlightuserdata(L, L);
	lua_gettable(L, LUA_REGISTRYINDEX);
	S = (lprofP_STATE*)lua_touserdata(L, -1);
	/* leave all functions under execution */
	while (lprofP_callhookOUT(S)) ;
	/* call the original Lua 'exit' function */
	lua_pushlightuserdata(L, &exit_id);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_call(L, 0, 0);
}

/* Our new coroutine.create function  */
/* Creates a new profile state for the coroutine */
static int coroutine_create(lua_State *L) {
  lprofP_STATE* S;
  lua_State *NL = lua_newthread(L);
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1), 1,
    "Lua function expected");
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  /* Inits profiler and sets profiler hook for this coroutine */
  S = lprofM_init();
  lua_pushlightuserdata(L, NL);
  lua_pushlightuserdata(L, S);
  lua_settable(L, LUA_REGISTRYINDEX);
  lua_sethook(NL, (lua_Hook)callhook, LUA_MASKCALL | LUA_MASKRET, 0);
  return 1;	
}

static int profiler_pause(lua_State *L) {
  lprofP_STATE* S;
  lua_pushlightuserdata(L, L);
  lua_gettable(L, LUA_REGISTRYINDEX);
  S = (lprofP_STATE*)lua_touserdata(L, -1);
  lprofM_pause_function(S);
  return 0;
}

static int profiler_resume(lua_State *L) {
  lprofP_STATE* S;
  lua_pushlightuserdata(L, L);
  lua_gettable(L, LUA_REGISTRYINDEX);
  S = (lprofP_STATE*)lua_touserdata(L, -1);
  lprofM_pause_function(S);
  return 0;
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

static const luaL_reg prof_funcs[] = {
	{ "pause", profiler_pause },
	{ "resume", profiler_resume },
	{ NULL, NULL }
};

int luaopen_profiler(lua_State *L) {
	lprofP_STATE* S;
    float function_call_time;

	function_call_time = calcCallTime(L);

    lua_sethook(L, (lua_Hook)callhook, LUA_MASKCALL | LUA_MASKRET, 0);
    /* init with default file name and printing a header line */
    if (!(S=lprofP_init_core_profiler(NULL, 1, function_call_time))) {
        printf("luaProfiler error: output file could not be opened!");
        exit(0);
    }

	lua_pushlightuserdata(L, L);
	lua_pushlightuserdata(L, S);
	lua_settable(L, LUA_REGISTRYINDEX);
	
    /* use our own exit function instead */
	lua_getglobal(L, "os");
	lua_pushlightuserdata(L, &exit_id);
	lua_pushstring(L, "exit");
	lua_gettable(L, -3);
	lua_settable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "exit");
	lua_pushcfunction(L, (lua_CFunction)exit_profiler);
	lua_settable(L, -3);

	/* use our own coroutine.create function instead */
/*	lua_getglobal(L, "coroutine");*/
/*	lua_pushstring(L, "create");*/
/*	lua_pushcfunction(L, (lua_CFunction)coroutine_create);*/
/*	lua_settable(L, -3);*/

	luaL_openlib(L, "profiler", prof_funcs, 0);

    /* the following statement is to simulate how the execution stack is */
    /* supposed to be by the time the profiler is activated when loaded  */
    /* as a library.                                                     */

    /* for this to be true, your Lua 5.0 environment must be started in  */
    /* a simmilar way to this:                                           */
    /* lua -e 'local f, e1, e2 = loadlib("./luaprofiler_lua50.so", "init_profiler") assert(f, (e1 or "").."\t"..(e2 or "")) f()' <normal_lua_starting_point.lua>   */

    /*lprofP_callhookIN(S, "", "(null)", "@init_profiler.lua", 0, -1);*/
	lprofP_callhookIN(S, "", "luaopen_profiler", "(C)", -1, -1);

	return 1;
}
