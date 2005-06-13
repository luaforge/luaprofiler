/*
** LuaProfiler 2.0
** Copyright Kepler Project 2005 (http://www.keplerproject.org/luaprofiler)
** $Id: lua32_profiler.c,v 1.4 2005-06-13 19:34:58 mascarenhas Exp $
*/

/*****************************************************************************
lua32_profiler.c:
   Lua version dependent profiler interface
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clocks.h"
#include "core_profiler.h"

#include "luadebug.h"
#include "lua.h"
#include "lualib.h"

lprofP_STATE *S;

/* called by Lua (via the callhook mechanism) */
static void callhook(lua_Function func, char *file, int line) {
char *source;
int linedefined;
char *func_name;

        if (lua_isfunction(func)) {
                /* entering a function */
                lua_funcinfo(func, &source, &linedefined);
                lua_getobjname(func, &func_name);
                lprofP_callhookIN(S, source, func_name, file, line, -1);
        }
        else {
                lprofP_callhookOUT(S);
        }
        
}


/* Lua function to exit politely the profiler                               */
/* redefines the lua exit() function to not break the log file integrity    */
/* The log file is assumed to be valid if the last entry has a stack level  */
/* of 1 (meaning that the function 'main' has been exited)                  */
static void exit_profiler() {
   /* leave all functions under execution */
   while (lprofP_callhookOUT(S)) ;
        /* call the original Lua 'exit' function */
        lua_callfunction(lua_getglobal("_exit"));
}


/* calculates the approximate time Lua takes to call a function */
static float calcCallTime() {
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
   lua_dostring(lua_code);
   return lprofC_get_seconds(timer) / (float) 100000;
}


void init_profiler(void *L) {
float function_call_time;

    function_call_time = calcCallTime();

    lua_setcallhook((lua_CHFunction)callhook);
    /* init with default file name and printing a header line */
    if (!(S=lprofP_init_core_profiler(NULL, 1, function_call_time))) {
        printf("luaProfiler error: output file could not be opened!");
        exit(0);
    }

    /* use our own exit function instead */
    lua_pushobject(lua_getglobal("exit"));
    lua_setglobal("_exit");
    lua_register("exit", exit_profiler);

}
