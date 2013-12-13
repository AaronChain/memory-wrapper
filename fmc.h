/*
 *  fmc.h
 *
 *  'Find Memory Corruption' related header file
 *
 *  Author: rp <rp@meetrp.com>
 *
 */

#ifndef __FMC_H__
#define __FMC_H__

#include <stdlib.h>
#include "fmc_common.h"

#define FMC_SIGNATURE		"0xC00BFACE"
#define FMC_SIGNATURE_SIZE	sizeof(FMC_SIGNATURE)
#define FMC_FUNCNAME_SIZE	16		/* in bytes */
#define FMC_LINENUM_SIZE	sizeof(int32_t)
#define FMC_HEADER_SIZE 	(FMC_SIGNATURE_SIZE + FMC_FUNCNAME_SIZE + FMC_LINENUM_SIZE)
#define FMC_FOOTER_SIZE 	FMC_HEADER_SIZE


void fmc_init();
void *fmc_alloc(const char *fname, const char *funct, int32_t line_no, size_t original_size);
void fmc_free(const char *fname, const char *funct, int32_t line_no, void *ptr);
void fmc_deinit();

#define FMC_INIT()			fmc_init()
#define FMC_ALLOC(s)		fmc_alloc(__FILE__, __FUNCTION__, __LINE__, s)
#define FMC_FREE(p)			fmc_free(__FILE__, __FUNCTION__, __LINE__, p)
#define FMC_DEINIT()		fmc_deinit()

#endif /* __FMC_H__ */
