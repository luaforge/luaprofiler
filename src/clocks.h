/*
** LuaProfiler 2.0
** Copyright Kepler Project 2005 (http://www.keplerproject.org/luaprofiler)
** $Id: clocks.h,v 1.3 2005-06-13 19:34:58 mascarenhas Exp $
*/

/*****************************************************************************
clocks.h:
   Module to register the time (seconds) between two events

Design:
   'lprofC_start_timer()' marks the first event
   'lprofC_get_seconds()' gives you the seconds elapsed since the timer
                          was started
*****************************************************************************/

#include <time.h>

void lprofC_start_timer(clock_t *time_marker);
float lprofC_get_seconds(clock_t time_marker);
