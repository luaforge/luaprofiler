#ifdef __cplusplus
extern "C" {
#endif

#include "cache.h"
#include <stdlib.h>
#include <string.h>

#define LPROF_CACHE_SIZE (1024*1024)
#define LPROF_LINE_SIZE (256)
#define LPROF_OUTPUT_LINE_SIZE (4096)

struct Cache
{
	char line_buf[LPROF_LINE_SIZE];
	char data[LPROF_CACHE_SIZE];
	size_t pos;
};

static struct Cache* cache;

void lprofCache_append(FILE* outfile, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	lprofCache_vappend(outfile, fmt, ap);
	va_end(ap);
}

void lprofCache_vappend(FILE* outfile, const char* fmt, va_list args)
{
	if (cache == NULL)
	{
		cache = (struct Cache*)malloc(sizeof(struct Cache));
	}

	vsprintf(cache->line_buf, fmt, args);
	size_t len = strlen(cache->line_buf);
	if (cache->pos + len >= LPROF_CACHE_SIZE - 1) /* leave a space for '\0' */
	{
		lprofCache_write_all(outfile);
	}
	
	memcpy(cache->data + cache->pos, cache->line_buf, len + 1); /* len+1 copy '\0' */
	cache->pos += len;
}

void lprofCache_write_all(FILE* outfile)
{
	size_t start = 0;
	while(cache->pos > 0)
	{
		size_t line_size = cache->pos < LPROF_OUTPUT_LINE_SIZE ?
								 cache->pos : LPROF_OUTPUT_LINE_SIZE;
		fwrite(cache->data + start, sizeof(char), line_size, outfile);
		cache->pos -= line_size;
		start += line_size;
	}
}

/* test cache */
/*
int main(int argc, char const *argv[])
{
	FILE* file = fopen("/tmp/test.out", "a");
	lprofCache_append(file, "abcdefg");
	lprofCache_append(file, "b\n");
	lprofCache_append(file, "c\n");

	lprofCache_write_all(file);
	fclose(file);

	return 0;
}
*/

#ifdef __cplusplus
}
#endif