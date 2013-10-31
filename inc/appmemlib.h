
#ifndef _APPMEMLIB_H
#define _APPMEMLIB_H
#ifndef _APPMEMD
#include <stdio.h>
#include <memory.h>
#endif
#include "am_test_os.h"

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


//#define AM_ASSERT(x) if(!x) {printf("ASSERT\n"); while(1);}
#define AM_ASSERT(x)


typedef unsigned int AM_RETURN;

#define AM_RET_GOOD         (0) 
#define AM_RET_PARAM_ERR    (1)
#define AM_RET_ALLOC_ERR    (2)
#define AM_RET_IO_ERR       (3)


typedef enum amTypeEnum 
{
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
	

}	AM_MEM_CAP_T;


typedef AM_RETURN (*am_fn)(UINT32 handle, void * p1, UINT64 l1, void *p2, UINT64 l2);
typedef AM_RETURN (*am_fn_align)(UINT32 handle, void * p1, void *p2);

/*
#define AM_FUNC_IDX_OPEN          0
#define AM_FUNC_IDX_CLOSE         1
#define AM_FUNC_IDX_READ          2
#define AM_FUNC_IDX_WRITE         3
#define AM_FUNC_IDX_ALIGNED_READ  4
#define AM_FUNC_IDX_ALIGNED_WRITE 6
#define AM_FUNC_IDX_COPY          7
#define AM_FUNC_IDX_SORT          8
*/

typedef struct _am_func_calls
{
	am_fn open;
	am_fn close;
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
	char name[64];
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



UINT32 am_get_capabilities_count(char *am_name);
AM_RETURN am_get_capabilities(char *am_name, AM_MEM_CAP_T *pAmCaps, UINT32 count);
UINT32 am_sprintf_capability(AM_MEM_CAP_T *pAmCap, char *buf, UINT32 buf_size);
AM_RETURN am_create_function(char *am_name, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc);
AM_FUNC_DATA_U * am_handle_to_funcdata(UINT32 handle);





#endif

