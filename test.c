/*
 *  test.c
 *
 *  'Find Memory Corruption' testing file
 *
 *  Author: rp <rp@meetrp.com>
 *
 */

#include <stdio.h>
#include "fmc.h"

int
main(void)
{
	char *a = NULL;

	FMC_INIT();
	a = (char *)FMC_ALLOC(10 * sizeof(char));

	/**
	 * uncomment accordingly for your tests.
	 **/ 
	//*(a+11) = 'a'; // overflow
	//*(a-1) = 'a'; // underflow
	//*(a-21) = 'a'; // corrupt a memory way below
	*(a-31) = 'a'; // corrupt a memory way above

	/* if you comment below it will result in memory leak */
	FMC_FREE(a);
	FMC_DEINIT();

	return 0;
}
