/*
 *  fmc_common.h
 *
 *  'Find Memory Corruption' commong header file
 *
 *  Author: rp <rp@meetrp.com>
 *
 */

#ifndef __FMC_COMMON_H__
#define __FMC_COMMON_H__

#include <stdio.h>
#include <stdint.h>
#include "fmc_log.h"

#define FMC_NULL_TERMINATE(str)		(str[strlen(str) - 1] = 0)

/*
typedef unsigned char 		int8_t;
typedef unsigned short		int16_t;
typedef unsigned int		int32_t;
typedef unsigned long long	int64_t;
*/

#endif /* __FMC_COMMON_H__ */
