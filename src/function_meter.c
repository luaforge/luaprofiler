/*****************************************************************************
function_meter.c:
   Module to compute the times for functions (local times and total times)

Design:
   'lprofM_init'            set up the function times meter service
   'lprofM_enter_function'  called when the function stack increases one level
   'lprofM_leave_function'  called when the function stack decreases one level

   'lprofM_resume_function'   called when the profiler is returning from a time
                              consuming task
   'lprofM_resume_total_time' idem
   'lprofM_resume_local_time' called when a child function returns the execution 
                              to it's caller (current function)
   'lprofM_pause_function'    called when the profiler need to do things that
                              may take too long (writing a log, for example)
   'lprofM_pause_total_time'  idem
   'lprofM_pause_local_time'  called when the current function has called
                              another one or when the function terminates
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "clocks.h"

/* #include "stack.h" is done by function_meter.h */
#include "function_meter.h"

#ifdef DEBUG
   #include <stdlib.h>
   #define ASSERT(e, msg) if (!e) {                                               \
                              fprintf(stdout,                                     \
                                      "function_meter.c: assertion failed: %s\n", \
                                      msg);                                       \
                              exit(1); }
#else
   #define ASSERT(e, msg)
#endif

/* the stack */
static lprofS_STACK stack_top;
/* structures to receive stack elements, declared globals */
/* in the hope they will perform faster                   */
static lprofS_STACK_RECORD newf;       /* used in 'enter_function' */
static lprofS_STACK_RECORD leave_ret;   /* used in 'leave_function' */



/* sum the seconds based on the time marker */ 
static void compute_local_time(lprofS_STACK_RECORD *e) {
   ASSERT(e, "local time null");
   e->local_time += lprofC_get_seconds(e->time_marker_function_local_time);
}


/* sum the seconds based on the time marker */ 
static void compute_total_time(lprofS_STACK_RECORD *e) {
   ASSERT(e, "total time null");
   e->total_time += lprofC_get_seconds(e->time_marker_function_total_time);
}


/* compute the local time for the current function */
void lprofM_pause_local_time() {
        compute_local_time(stack_top);
}


/* pause the total timer for all the functions that are in the stack */
void lprofM_pause_total_time() {
lprofS_STACK aux;

   ASSERT(stack_top, "pause_total_time: stack_top null");

   /* auxiliary stack */
        aux = stack_top;
        
        /* pause */
        while (aux) {
                compute_total_time(aux);
                aux = aux->next;
        }
}


/* pause the local and total timers for all functions in the stack */
void lprofM_pause_function() {

   ASSERT(stack_top, "pause_function: stack_top null");

        lprofM_pause_local_time();
        lprofM_pause_total_time();
}


/* resume the local timer for the current function */
void lprofM_resume_local_time() {

   ASSERT(stack_top, "resume_local_time: stack_top null");
                
   /* the function is in the top of the stack */
        lprofC_start_timer(&(stack_top->time_marker_function_local_time));
}


/* resume the total timer for all the functions in the stack */
void lprofM_resume_total_time() {
lprofS_STACK aux;

   ASSERT(stack_top, "resume_total_time: stack_top null");
        
   /* auxiliary stack */
        aux = stack_top;
        
        /* resume */
        while (aux) {
                lprofC_start_timer(&(aux->time_marker_function_total_time));
                aux = aux->next;
        }
}


/* resume the local and total timers for all functions in the stack */
void lprofM_resume_function() {

   ASSERT(stack_top, "resume_function: stack_top null");

        lprofM_resume_local_time();
        lprofM_resume_total_time();
}


/* the local time for the parent function is paused  */
/* and the local and total time markers are started */
void lprofM_enter_function(char *file_defined, char *fcn_name, long linedefined, long currentline) {
        /* the flow has changed to another function: */
   /* pause the parent's function timer timer   */
   if (stack_top)
        lprofM_pause_local_time();
        /* measure new function */
        lprofC_start_timer(&(newf.time_marker_function_local_time));
        lprofC_start_timer(&(newf.time_marker_function_total_time));
        newf.file_defined = file_defined;
        newf.function_name = fcn_name;
        newf.line_defined = linedefined;
   newf.current_line = currentline;
        newf.local_time = 0.0;
        newf.total_time = 0.0;
        lprofS_push(&stack_top, newf);
}


/* computes times and remove the top of the stack         */
/* 'isto_resume' specifies if the parent function's timer */
/* should be restarted automatically. If it's false,      */
/* 'resume_local_time()' must be called when the resume   */
/* should be done                                         */
/* returns the funcinfo structure                         */
/* warning: use it before another call to this function,  */
/* because the funcinfo will be overwritten               */
lprofS_STACK_RECORD *lprofM_leave_function(int isto_resume) {

   ASSERT(stack_top, "leave_function: stack_top null");

        leave_ret = lprofS_pop(&stack_top);
        compute_local_time(&leave_ret);
        compute_total_time(&leave_ret);
        /* resume the timer for the parent function ? */
        if (isto_resume)
                lprofM_resume_local_time();
        return &leave_ret;
}


/* init stack */
void lprofM_init() {
   stack_top = NULL;
}
