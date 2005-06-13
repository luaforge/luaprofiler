/*
** LuaProfiler 2.0
** Copyright Kepler Project 2005 (http://www.keplerproject.org/luaprofiler)
** $Id: cgilua32_profiler.c,v 1.3 2005-06-13 19:34:58 mascarenhas Exp $
*/

/*****************************************************************************
cgilua32_profiler.c:
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


/* called by Lua (via the callhook mechanism) */
static void callhook(lua_State *L, lua_Function func, char *file, int line) {
char *source;
int linedefined;
char *func_name;

        if (lua_isfunction(L, func)) {
                /* entering a function */
                lua_funcinfo(L, func, &source, &linedefined);
                lua_getobjname(L, func, &func_name);
                lprofP_callhookIN(source, func_name, file, line, -1);
        }
        else {
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
        lua_callfunction(L, lua_getglobal(L, "_exit"));
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


void init_profiler(lua_State *L) {
float function_call_time;

   function_call_time = calcCallTime(L);

    lua_setcallhook(L, (lua_CHFunction)callhook);
    /* init with default file name and printing a header line */
    if (!lprofP_init_core_profiler(NULL, 1, function_call_time)) {
        printf("Content-type: text/html\n\n");
        printf("luaProfiler error: output file could not be opened!");
        exit(0);
    }

    /* use our own exit function instead */
    lua_pushobject(L, lua_getglobal(L, "exit"));
    lua_setglobal(L, "_exit");
    lua_register(L, "exit", exit_profiler);


    /* the two following statements are to simulate how the execution stack  */
    /* is supposed to be by the time the profiler is activated in the cgilua */
    /* environment - in the cgilua environment it is not possible to open a  */
    /* custom library before the Lua main file has been started. */

    /* for this to be true, it is user's resposibility to place this line in */
    /* the first line of the 'cgilua.lua' file:                              */
    /* callfromlib(loadlib('profiler', './cgilua.conf/'), 'init_profiler')   */

    lprofP_callhookIN("", "(null)", "@cgilua.lua", 0, -1);
    lprofP_callhookIN("", "callfromlib", "(C)", -1, -1);

}


