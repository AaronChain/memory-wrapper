/*
 *  fmc_log.h
 *
 *  'Find Memory Corruption' logging related header file
 *
 *  Author: rp <rp@meetrp.com>
 *
 */

#ifndef __FMC_LOG_H__
#define __FMC_LOG_H__

#include "fmc_common.h"

#define FMC_LOG_FILE		"/tmp/fmc.log"
#define FMC_LOG_MAX_LEN		1024

//#ifdef ENABLE_FMC_LOG
	void fmc_log(const char *log_level, const char *fname, const char *func, int32_t line_no, const char *fmt, ...);
	void fmc_back_trace(const char *fname, const char *func, int32_t line_no);
	#define fmc_dbg(...)	fmc_log("DBG", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define fmc_err(...)	fmc_log("ERR", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
	#define fmc_fatal()												\
	{																\
		fmc_back_trace(__FILE__, __FUNCTION__, __LINE__);			\
		exit(EXIT_FAILURE);											\
	}
	/*
#else
	#define fmc_dbg(...)
	#define fmc_err(...)
	#define fmc_fatal()
#endif
*/

#endif /* __FMC_LOG_H__ */
