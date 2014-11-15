/*
** LuaProfiler
** Jennal
** 2014-11-15
*/

/*****************************************************************************
cache.h:
   Module to faster write file

Design:
   'lprofCache_append()' append string to cache
   'lprofCache_vappend()' append string to cache with va_list
   'lprofCache_write_all()' write all cache data to output and clear cache
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>

void lprofCache_append(FILE* outfile, const char* fmt, ...);
void lprofCache_vappend(FILE* outfile, const char* fmt, va_list args);
void lprofCache_write_all(FILE* outfile);
    
#ifdef __cplusplus
}
#endif
