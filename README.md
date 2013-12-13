Find Memory Corruption
======================

FMC, as I fondly call it, is a small wrapper that I wrote around malloc() and free() that will help me find if there are any memory leaks or memory corruptions.


LOGIC
=====

Each memory allocation requests results in additional memory sought and the user memory padded with additional debug information. This padding takes the following format:

	---------------------------------------------------------------------------------------------------
	| ORIGINAL SIZE | LINENO | FUNC_NAME | SIGNATURE | <USER MEMORY> | SIGNATURE | FUNC_NAME | LINENO |
	---------------------------------------------------------------------------------------------------

	where,
		ORIGINAL SIZE	- User request size,
		LINENO			- line # from where this alloc was called
		FUNC_NAME		- function from where this alloc was called
		SIGNATURE		- at the moment it is 0xC00BFACE.
		<USER MEMORY>	- what is available for user to use.

With this above padding, any overflow or underflow error can be caught. Apart from this I also maintain a linked-list of all allocated memory and this list will checked for allocated-but-not-freed memory addresses (i.e., memory leaks) just before shut-down.


USAGE
=====
	0. Obviously, the zeroth step is to include "fmc.h"
	1. First call, FMC_INIT() to initialize the linked-list related backend.
	2. Next, you can start allocating memory using the wrapper FMC_ALLOC().
	3. Post usage, use this FMC_FREE() to free the memory.
	4. Finally, just before exit, invoke FMC_DEINIT() to cross check if there are any memory leaks.
	5. Now include FMC files into your Makefile or add your files into FMC makefile.


LOGs
====
At the moment, all the logging is sent to a file "/tmp/fmc.log" and this takes the following format.

Fri Dec 13 22:05:17 2013 [fmc.c            : fmc_init         :   31 ] : [DBG] allocation table has been initialized
Fri Dec 13 22:05:17 2013 [fmc.c            : fmc_alloc        :  188 ] : [DBG] allocated memory[0x185b2f0] for a call from [main:19]
Fri Dec 13 22:05:17 2013 [fmc.c            : fmc_deinit       :   54 ] : [ERR] ===================== ERROR =====================
Fri Dec 13 22:05:17 2013 [fmc.c            : fmc_deinit       :   55 ] : [ERR] allocation table is not empty!!
Fri Dec 13 22:05:17 2013 [fmc.c            : fmc_deinit       :   56 ] : [ERR] Leaked memory addresses are:
Fri Dec 13 22:05:17 2013 [fmc.c            : fmc_deinit       :   76 ] : [ERR] 0x185b2f0 --> HDR[main:97] - FTR[main:19]
Fri Dec 13 22:05:17 2013 [fmc.c            : fmc_deinit       :   79 ] : [ERR] 
