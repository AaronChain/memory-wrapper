/*
 *  fmc.c
 *
 *  'Find Memory Corruption' related main file
 *
 *  Author: rp <rp@meetrp.com>
 *
 */

#include "fmc.h"
#include <string.h>

typedef struct __fmc_mem_table {
	int64_t allocated_addr;
	struct __fmc_mem_table *prev;
	struct __fmc_mem_table *next;
} fmc_mem_table_t;
static fmc_mem_table_t *fmc_allocation_table_head;


/**
 * fmc_init()
 *
 * initialize the backend for future debugging
 *
 **/
void
fmc_init()
{
	fmc_allocation_table_head = NULL;
	fmc_dbg("allocation table has been initialized");
}


/**
 * fmc_deinit()
 *
 * Deinitialize the backend for future debugging
 *
 **/
void
fmc_deinit()
{
	fmc_mem_table_t *iterator = NULL;
	char *hdr_func, *ftr_func;
	int32_t hdr_lineno, ftr_lineno;
	int32_t posn = 0;
	size_t orig_size;
	void *ptr;

	if (NULL == fmc_allocation_table_head)
		return;

	fmc_err("===================== ERROR ====================="); 
	fmc_err("allocation table is not empty!!");
	fmc_err("Leaked memory addresses are:");
	iterator = fmc_allocation_table_head;
	while (iterator) {
		ptr = (void *)iterator->allocated_addr;
		posn = 0;		
		orig_size = *(size_t *)(ptr + posn); posn += sizeof(size_t);

		// extract the header
		hdr_lineno = *(int32_t *)(ptr + posn); posn += FMC_LINENUM_SIZE;
		hdr_func = (char *)(ptr + posn); posn += FMC_FUNCNAME_SIZE;

		posn += FMC_SIGNATURE_SIZE;	// add the header signature size
		posn += orig_size;			// add the original size
		posn += FMC_SIGNATURE_SIZE;	// add the footer signature size

		// extract the header
		ftr_func = (char *)(ptr + posn); posn += FMC_FUNCNAME_SIZE;
		ftr_lineno = *(int32_t *)(ptr + posn); posn += FMC_LINENUM_SIZE;

		fmc_err("%p --> HDR[%s:%ld] - FTR[%s:%d]", ptr, 
				hdr_func, hdr_lineno, ftr_func, ftr_lineno);
		iterator = iterator->next;
	}
	fmc_err("");
}


/**
 * fmc_add_new_elem
 *
 * allocate a new element to be put in the table
 *
 **/
static void
fmc_add_new_elem(void *ptr)
{
	fmc_mem_table_t *new_elem = (fmc_mem_table_t *)calloc(1, sizeof(fmc_mem_table_t));;
	new_elem->allocated_addr = (int64_t)ptr;

	if (NULL == fmc_allocation_table_head) {
		fmc_allocation_table_head = new_elem;
	} else {
		new_elem->prev = fmc_allocation_table_head;
		fmc_allocation_table_head->next = new_elem;
	}
}


/**
 * fmc_find_elem
 *
 * find an element in the table
 *
 **/
static fmc_mem_table_t*
fmc_find_elem(void *ptr) 
{
	fmc_mem_table_t *iterator = fmc_allocation_table_head;
	while (iterator) {
		if (iterator->allocated_addr == (int64_t)ptr) break;
		iterator = iterator->next;
	}

	return iterator;
}


/**
 * fmc_del_elem
 *
 * delete an element from the table
 *
 **/
static void
fmc_del_elem(void *ptr)
{
	fmc_mem_table_t *elem = fmc_find_elem(ptr);
	if (elem == fmc_allocation_table_head) fmc_allocation_table_head = NULL;
	if (NULL != elem->prev) elem->prev->next = elem->next;
	if (NULL != elem->next) elem->next->prev = elem->prev;

	free(elem);
}


/**
 * fmc_alloc()
 *
 * allocate memory for the user with additional debug information
 *
 *  fname		- file name for this invocation.
 *  func		- name of the function where this was invoked
 *  line_no		- line number of the function where this was invoked
 *  size		- size to be allocated
 *
 * The final outcome is:
 * 		---------------------------------------------------------------------------------------------------
 * 		| ORIGINAL SIZE | LINENO | FUNC_NAME | SIGNATURE | <USER MEMORY> | SIGNATURE | FUNC_NAME | LINENO |
 * 		---------------------------------------------------------------------------------------------------
 **/
void*
fmc_alloc(const char *fname, const char *func, int32_t line_no, size_t original_size)
{
	size_t new_size = sizeof(size_t) + FMC_HEADER_SIZE + original_size + FMC_FOOTER_SIZE;
	char *buf = calloc(new_size, sizeof(char));
	int32_t posn = 0;

	/*
	 * Fill up the HEADER with:
	 * 		-----------------------------------------------------------------------
	 * 		| ORIGINAL SIZE | LINENO | FUNC_NAME | SIGNATURE | user area.....
	 * 		-----------------------------------------------------------------------
	 */
	*((size_t *)(buf + posn)) = original_size; posn += sizeof(size_t);
	*((int32_t *)(buf + posn)) = line_no; posn += FMC_LINENUM_SIZE;
	snprintf((char*)buf + posn, (size_t)FMC_FUNCNAME_SIZE, "%s", func); posn += FMC_FUNCNAME_SIZE;
	snprintf((char*)buf + posn, (size_t)FMC_SIGNATURE_SIZE, "%s", FMC_SIGNATURE); posn += FMC_SIGNATURE_SIZE;

	// add up the original size
	posn += original_size;

	/*
	 * Fill up the FOOTER with:
	 * 		---------------------------------------------------
	 * 		     ...user area| SIGNATURE | FUNC_NAME | LINENO |
	 * 		---------------------------------------------------
	 */
	snprintf((char*)buf + posn, (size_t)FMC_SIGNATURE_SIZE, "%s", FMC_SIGNATURE); posn += FMC_SIGNATURE_SIZE;
	snprintf((char*)buf + posn, (size_t)FMC_FUNCNAME_SIZE, "%s", func); posn += FMC_FUNCNAME_SIZE;
	*((int32_t *)(buf + posn)) = line_no;

	fmc_add_new_elem(buf);
	fmc_dbg("allocated memory[%p] for a call from [%s:%d]", buf, func, line_no);
	return (buf + sizeof(size_t) + FMC_HEADER_SIZE);
}


/**
 * fmc_check_memory
 *
 * check for memory corruption
 **/
static void
fmc_check_memory(void *ptr, const char *fname, const char *func, int32_t line_no)
{
	int32_t posn = 0;
	size_t orig_size;
	char *hdr_sign, *ftr_sign;
	char *hdr_func, *ftr_func;
	int32_t hdr_lineno, ftr_lineno;

	// extract the original size
	orig_size = *(size_t *)(ptr + posn); posn += sizeof(size_t);

	// extract the header
	hdr_lineno = *(int32_t *)(ptr + posn); posn += FMC_LINENUM_SIZE;
	hdr_func = (char *)(ptr + posn); posn += FMC_FUNCNAME_SIZE;
	hdr_sign = (char *)(ptr + posn); posn += FMC_SIGNATURE_SIZE;

	// add the original size
	posn += orig_size;

	// extract the header
	ftr_sign = (char *)(ptr + posn); posn += FMC_SIGNATURE_SIZE;
	ftr_func = (char *)(ptr + posn); posn += FMC_FUNCNAME_SIZE;
	ftr_lineno = *(int32_t *)(ptr + posn); posn += FMC_LINENUM_SIZE;

	// Check SIGNATUREs
	if (memcmp((const void*)hdr_sign, FMC_SIGNATURE, FMC_SIGNATURE_SIZE)) {
		fmc_err("===================== ERROR ====================="); 
		fmc_err("Underflow error on memory allocated by [%s:%d] & freed at [%s:%d]",
					ftr_func, ftr_lineno, func, line_no);
		fmc_fatal();
	}
	if (memcmp((const void*)ftr_sign, FMC_SIGNATURE, FMC_SIGNATURE_SIZE)) {
		fmc_err("===================== ERROR ====================="); 
		fmc_err("Overflow error on memory allocated by [%s:%d] & freed at [%s:%d]",
					hdr_func, hdr_lineno, func, line_no);
		fmc_fatal();
	}

	// Check the rest
	if (memcmp((const void*)hdr_func, (const void*)ftr_func, FMC_FUNCNAME_SIZE)) {
		fmc_err("===================== ERROR ====================="); 
		fmc_err("Memory corruption at function name on memory allocated by [%s:%d] & freed at [%s:%d]",
					hdr_func, hdr_lineno, func, line_no);
		fmc_err("Memory corruption at function name on memory allocated by [%s:%d] & freed at [%s:%d]",
					ftr_func, ftr_lineno, func, line_no);
		fmc_fatal();
	}

	if (hdr_lineno != ftr_lineno) {
		fmc_err("===================== ERROR ====================="); 
		fmc_err("Memory corruption at line number on memory allocated by [%s:%d] & freed at [%s:%d]",
					hdr_func, hdr_lineno, func, line_no);
		fmc_err("Memory corruption at line number on memory allocated by [%s:%d] & freed at [%s:%d]",
					ftr_func, ftr_lineno, func, line_no);
		fmc_fatal();
	}

	fmc_dbg("All looks well with %p by [%s:%d]", ptr, hdr_func, hdr_lineno);
}


/**
 * fmc_free()
 *
 * frees the memory provided to the user through this framework
 *
 *  fname		- file name for this invocation.
 *  func		- name of the function where this was invoked
 *  line_no		- line number of the function where this was invoked
 *  ptr			- pointer to be freed
 *
 **/
void
fmc_free(const char *fname, const char *func, int32_t line_no, void *ptr)
{
	char *our_ptr = (char *)ptr - (FMC_HEADER_SIZE + sizeof(size_t));

	fmc_check_memory(our_ptr, fname, func, line_no);
	fmc_del_elem(our_ptr);
	fmc_dbg("freeing up memory[%p] for a call from [%s:%d]", our_ptr, func, line_no);
}
