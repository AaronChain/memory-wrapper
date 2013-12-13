/*
 *  fmc_long.c
 *
 *  'Find Memory Corruption' logging related file
 *
 *  Author: rp <rp@meetrp.com>
 *
 */

#include <stdarg.h>		/* va_*() */
//#include <stdio.h>		/* fopen(), FILE, ... */
#include <time.h>		/* time(), ctime(), ... */
#include <string.h>		/* strlen() */
#include <unistd.h>		/* getpid() */
#include <pthread.h>	/* pthread_self() */
#include <execinfo.h>	/* backtrace() */

#include "fmc.h"


/**
 * fmc_log()
 *
 * Logs the msg into the file defined, in 'fmc.h', by LOG_FILE.
 *
 *  log_level  - ERR, DBG, INF, etc... all of 3 chars only
 *  fname      - file name for this invocation.
 *  func       - name of the function where this was invoked
 *  line_no    - line number of the function where this was invoked
 *  fmt        - printf format specifier
 *
 **/
void
fmc_log(const char *log_level, const char *fname, const char *func, int32_t line_no, const char *fmt, ...)
{
	va_list args;
	char msg[FMC_LOG_MAX_LEN];
	time_t timer;
	char *cur_time_str = NULL;

	FILE *fstream = fopen(FMC_LOG_FILE, "a");
	if (NULL == fstream)
		fstream = stdout;

	timer = time(NULL);
	cur_time_str = ctime(&timer);
	FMC_NULL_TERMINATE(cur_time_str);


	va_start(args, fmt);
	vsnprintf(msg, FMC_LOG_MAX_LEN, fmt, args);
	va_end(args);

#if 0
    fprintf(fstream,
				"%-24.24s [%-6d:%16ld] %-16.16s %-16.16s %4d : [%.3s] %s\n",
				cur_time_str, getpid(), pthread_self(), fname, func, line_no,
				log_level, msg);
#endif
    fprintf(fstream,
				"%-24.24s [%-16.16s : %-16.16s : %4d ] : [%.3s] %s\n",
				cur_time_str, fname, func, line_no, log_level, msg);

	fclose(fstream);
}

/**
 * fmc_back_trace()
 *
 * print back trace to the log
 *
 *  fname		- file name for this invocation.
 *  func		- name of the function where this was invoked
 *  line_no		- line number of the function where this was invoked
 *
 **/
void
fmc_back_trace(const char *fname, const char *func, int32_t line_no)
{
	void *buf[255];
	size_t size;
	char **strings;
	size_t i;
	
	size = backtrace(buf, sizeof(buf)/sizeof(void*));
	strings = backtrace_symbols(buf, size);
	for (i = 0; i < size; i++)
		fmc_log("FTL", fname, func, line_no, "%s", strings[i]);

	fmc_log("FTL", fname, func, line_no, "");
	free(strings);
}
