/*****************************************************************************
core_profiler.h:
   Lua version independent profiler interface.
   Responsible for handling the "enter function" and "leave function" events
   and for writing the log file.

Design (using the Lua callhook mechanism) :
   'lprofP_init_core_profiler' set up the profile service
   'lprofP_callhookIN'         called whenever Lua enters a function
   'lprofP_callhookOUT'        called whenever Lua leaves a function
*****************************************************************************/


/* computes new stack and new timer */
void lprofP_callhookIN(char *source, char *func_name, char *file, int linedefined, int currentline);

/* pauses all timers to write a log line and computes the new stack */
/* returns if there is another function in the stack */
int  lprofP_callhookOUT();

/* opens the log file */
/* returns true if the file could be opened */
int lprofP_init_core_profiler(char *_out_filename, int isto_printheader, float _function_call_time);

