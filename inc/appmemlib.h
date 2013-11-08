/*****************************************************************************************

Copyright (c) 2013, Brandon Awbrey
All rights reserved.

https://github.com/scuzzydude/appmem



Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************************/

#ifndef _APPMEMLIB_H
#define _APPMEMLIB_H
#ifndef _APPMEMD
#include <stdio.h>
#include <memory.h>
#endif
#include "am_test_os.h"

typedef int AM_RETURN;

typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;
#ifndef NULL
#define NULL (void *)0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#include "appmemd_ioctl.h"

#ifdef _APPMEMD
#define AM_ASSERT(x) if(!x) printk(KERN_NOTICE "appmemk: ASSERT  File=%s Line=%d", __FILE__, __LINE__)
#else
#define AM_ASSERT(x) if(!x) {printf("ASSERT\n"); while(1);}
#endif

//#define AM_ASSERT(x)



#define AM_RET_GOOD             (0) 
#define AM_RET_PARAM_ERR        (1)
#define AM_RET_ALLOC_ERR        (2)
#define AM_RET_IO_ERR           (3)
#define AM_RET_OPEN_ERR         (4)
#define AM_RET_KEY_NOT_FOUND    (5)
#define AM_RET_INVALID_FUNC     (6)
#define AM_RET_INVALID_HDL      (7)
#define AM_RET_KEY_OUT_OF_RANGE (8)
typedef enum amTypeEnum 
{
	AM_TYPE_BASE_APPMEM,
	AM_TYPE_FLAT_MEM,
	AM_TYPE_ARRAY, 
	AM_TYPE_ASSOC_ARRAY
	
} AM_TYPE_ENUM;



typedef struct am_mem_cap_t
{
	AM_TYPE_ENUM amType;
	UINT64 maxSize;
	UINT32 functionCount;
	UINT32 typeSpecific[32];	
	UINT32 acOps[2];

}	AM_MEM_CAP_T;


#if 0
typedef AM_RETURN (*am_fn)(AM_HANDLE handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
typedef AM_RETURN (*am_fn_align)(AM_HANDLE handle, void * p1, void *p2);
typedef AM_RETURN (*am_fn_raw)(void * p1);
#endif

typedef struct _am_func_calls
{
	am_fn_raw open;
	am_fn_raw close;
	am_fn read;
	am_fn write;
	am_fn_align read_al;
	am_fn_align write_al;
	am_fn copy;
	am_fn sort;
	am_fn *fn;


} AM_FUNC_CALLS_T;


typedef struct am_mem_function_t
{
	UINT32 handle;
	AM_MEM_CAP_T amCap;
	APPMEM_RESP_CR_FUNC_T crResp;
	AM_FUNC_CALLS_T *fn;

} AM_MEM_FUNCTION_T;

/* PRIVATE */
typedef union am_func_data
{
	struct
	{
		void *data;
		UINT64 size;
	} common;
	struct
	{
		void *data;
		UINT64 size;
		UINT32 add_size;

	} flat;
	struct
	{
		void *data;
		UINT64 size;
	    UINT32 idx_size;
		UINT32 data_size;

	} stata; /* Static Array */
	struct
	{
		void *data;
		UINT64 size;
		UINT64 cur_count;
	} assca;


} AM_FUNC_DATA_U;


typedef struct _am_command
{
    UINT32 cmd;
    UINT32 len;

} AM_COMMAND_T;



#define TS_FLAT_ADDRESS_BYTE_SIZE   0
#define TS_FLAT_SIZE                1
#define TS_MIN_BYTE_ACTION          2
#define TS_MAX_BYTE_ACTION          3
#define TS_INIT_MEM_VAL             4

#define TS_STAT_ARRAY_IDX_BYTE_SIZE  0 
#define TS_STAT_ARRAY_VAL_MIN_SIZE   1 
#define TS_STAT_ARRAY_VAL_MAX_SIZE   2 
#define TS_STAT_ARRAY_VAL_DATA_TYPES 3
#define TS_STAT_DT_FIXED_WIDTH   0x1
#define TS_STAT_DT_VAR_WIDTH     0x2

#define TS_ASSCA_KEY_MAX_SIZE    0 
#define TS_ASSCA_DATA_MAX_SIZE   1 
#define TS_ASSCA_KEY_TYPE        2
#define TS_ASSCA_KEY_FIXED_WIDTH   0x1
#define TS_ASSCA_KEY_VAR_WIDTH     0x2

#define TS_ASSCA_DATA_TYPE        2
#define	TS_ASSCA_DATA_FIXED_WIDTH  0x1
#define TS_ASSCA_DATA_VAR_WIDTH    0x2


typedef struct amlib_entry_
{
	char			   *am_name;
	int                fh;
	UINT32             (*get_cap_count)(struct amlib_entry_ *pEntry);
	AM_RETURN		   (*get_capabilities)(struct amlib_entry_ *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count);
	AM_RETURN          (*create_function)(struct amlib_entry_ *pEntry, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc);
} AMLIB_ENTRY_T;


UINT32 am_sprintf_capability(AM_MEM_CAP_T *pAmCap, char *buf, UINT32 buf_size);

AM_FUNC_DATA_U * am_handle_to_funcdata(UINT32 handle);

AM_RETURN am_get_entry_point(char *am_name, AMLIB_ENTRY_T *pEntry);






#endif

