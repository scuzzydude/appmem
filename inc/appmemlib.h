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

#define AM_VER_MAJOR  0
#define AM_VER_MINOR  1
#define AM_VER_BUILD  1
#define AM_VER_BUGFIX 0

typedef union _am_version_u
{
	UINT32 dwVersion;
	struct 
	{
		UINT8 major;
		UINT8 minor;
		UINT8 build;
		UINT8 bugfix;

	} version;

} AM_VERSION_U;





#define AM_RET_GOOD             (0) 
#define AM_RET_PARAM_ERR        (1)
#define AM_RET_ALLOC_ERR        (2)
#define AM_RET_IO_ERR           (3)
#define AM_RET_OPEN_ERR         (4)
#define AM_RET_KEY_NOT_FOUND    (5)
#define AM_RET_INVALID_FUNC     (6)
#define AM_RET_INVALID_HDL      (7)
#define AM_RET_KEY_OUT_OF_RANGE (8)
#define AM_RET_SOCKET_ERR       (9)
#define AM_RET_IO_UNDERRUN     (10)
#define AM_RET_THREAD_ERR      (11)
#define AM_RET_Q_OVERFLOW      (12)
#define AM_RET_INVALID_PACK_OP (13)
#define AM_RET_INVALID_DEV     (14)
#define AM_RET_INVALID_OPCODE  (15)
#define AM_RET_TAG_NOT_FOUND   (16)
#define AM_RET_NO_DEVICE_SLOT  (17)

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


typedef struct _am_func_calls
{
	am_fn_raw open;
	am_fn_raw close;
	am_fn read;
	am_fn write;
	am_fn_align read_al;
	am_fn_align write_al;
	am_fn copy;
	am_fn_action sort;
	am_fn *fn;


} AM_FUNC_CALLS_T;


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
		UINT32 array_size;

	} stata; /* Static Array */
	struct
	{
		void *data;
		UINT64 size;
		UINT64 cur_count;
	} assca;


} AM_FUNC_DATA_U;

/* Sort Types */
#define AM_SORT_TYPE_STATA_INTEGRAL_MERGE    1

/* Sort Orders */
#define AM_SORT_ORDER_ASC			         0
#define AM_SORT_ORDER_DESC                   1



typedef union am_sort_param
{
	struct
	{
		UINT32 type;

	} common;
	struct
	{
		UINT32 type;
		UINT32 start_idx;
		UINT32 end_idx;
		UINT32 order;
		UINT8  data_signed; /* flags, can be bit fields if we think of more of them */	

	} stata_integral;


} AM_SORT_PARAM_U;





typedef struct am_mem_function_t
{
	UINT32 handle;
	AM_MEM_CAP_T amCap;
	APPMEM_RESP_CR_FUNC_T crResp;
	AM_FUNC_CALLS_T *fn;
	AM_FUNC_DATA_U *pVdF;
    AM_FN_U             *pfnOps;     

} AM_MEM_FUNCTION_T;


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
	void               *pTransport;
    void               *pTarget;
	void               *pThread;
} AMLIB_ENTRY_T;

#define AM_FUNC_PACK_TYPE_FLAG_RESP    0x8000
#define AM_FUNC_PACK_TYPE_FLAG_ERR     0x4000

#define AM_PACK_TYPE_OPCODE_ONLY       0x0001
#define AM_PACK_TYPE_FIVO              0x0002

#define AM_PACK_FUNC_ID_BASEAPPMEM     0xFFFA


typedef struct _am_pack_wrapper
{
	UINT16 func_id;
	UINT16 packType;
	UINT16 appTag;
	UINT16 size; /* Includes wrapper size of 8 */
	UINT32 op;
} AM_PACK_WRAPPER_T; 



#define MAX_BASIC_PACK_UNION_SIZE 512


typedef struct _am_pack_identify
{
	AM_PACK_WRAPPER_T wrap;

} AM_PACK_IDENTIFY;

typedef struct _am_pack_get_cap_count
{
	AM_PACK_WRAPPER_T wrap;

} AM_PACK_GET_CAP_COUNT;

typedef struct _am_pack_read_align
{
	AM_PACK_WRAPPER_T   wrap;
	UINT64              p1;

} AM_PACK_READ_ALIGN;

typedef struct _am_pack_fixed_in_var_out
{
	AM_PACK_WRAPPER_T   wrap;
	UINT64              l1;
	UINT32              data_in[1];

} AM_PACK_FIXED_IN_VAR_OUT;



typedef union _am_pack_all_u
{
	AM_PACK_WRAPPER_T			wrap;
    AM_PACK_IDENTIFY			op;
	AM_PACK_GET_CAP_COUNT		cap_count;
	AM_PACK_READ_ALIGN			read_al;
	AM_PACK_FIXED_IN_VAR_OUT    fivo;

	UINT8                   raw[MAX_BASIC_PACK_UNION_SIZE];
} AM_PACK_ALL_U;



typedef struct _am_pack_resp_err
{
	AM_PACK_WRAPPER_T     wrap;
	AM_RETURN           status;

} AM_PACK_RESP_ERR;

typedef struct _am_pack_resp_identify
{
	AM_PACK_WRAPPER_T      wrap;
	UINT32                 amType;
	AM_VERSION_U           amVersion;


} AM_PACK_RESP_IDENTIFY;


typedef struct _am_pack_resp_cap_count
{
	AM_PACK_WRAPPER_T      wrap;
	UINT32                 cap_count;


} AM_PACK_RESP_CAP_COUNT;

typedef struct _am_pack_resp_align
{
	AM_PACK_WRAPPER_T      wrap;
	UINT8                  resp_bytes[MAX_BASIC_PACK_UNION_SIZE - sizeof(AM_PACK_WRAPPER_T)];


} AM_PACK_RESP_ALIGN;

typedef struct _am_pack_resp_cr_func
{
	AM_PACK_WRAPPER_T      wrap;
	APPMEM_RESP_CR_FUNC_T crResp;

} AM_PACK_RESP_CR_FUNC;

typedef union _am_pack_resp_u
{
	AM_PACK_WRAPPER_T        wrap;
	AM_PACK_RESP_ERR         error;
	AM_PACK_RESP_IDENTIFY    identify;
	AM_PACK_RESP_CAP_COUNT   cap_count;
	AM_PACK_RESP_ALIGN       align_resp;
	AM_PACK_RESP_CR_FUNC     crResp;

	UINT8                    raw[MAX_BASIC_PACK_UNION_SIZE];

} AM_PACK_RESP_U;



typedef struct _am_recv_cmd
{
	AM_PACK_ALL_U         *pRxBuf;
	void                  *pvClient;
	UINT32                recv_len;
	UINT32                state;
	UINT32                idx;
	UINT32                buf_count;
	struct _am_recv_cmd   *next;

} AM_REC_CMD_T;

#define RX_CMD_STATE_FREE        (0)
#define RX_CMD_STATE_ACTIVE      (1)
#define RX_CMD_STATE_CONT_BUF_2  (2) /* 3... 4...5... */ 

/* The idea is a single lock-less TX/RX queue */
/* The data buffers are continguous, so if we over run we just use the next segment */

typedef struct _am_pack_queue
{
	AM_REC_CMD_T *pRxCmd;

    AM_PACK_ALL_U   *pRxBuf;
    AM_PACK_RESP_U  *pTxBuf;
    UINT32  size;
    UINT32  rx_ci;
    UINT32  rx_pi;
    UINT32  tx_ci;
    UINT32  tx_pi;
    

} AM_PACK_QUEUE_T;


UINT32 am_sprintf_capability(AM_MEM_CAP_T *pAmCap, char *buf, UINT32 buf_size);
AM_FUNC_DATA_U * am_handle_to_funcdata(UINT32 handle);
AM_RETURN am_get_entry_point(char *am_name, AMLIB_ENTRY_T *pEntry);
AM_PACK_QUEUE_T * am_init_pack_queue(UINT32 pack_count);





static AM_MEM_CAP_T virtd_caps[] = 
{

	{ 
		AM_TYPE_BASE_APPMEM,
		(1024 * 1024 * 128), /* Total Device Memory */
		2,
		{
			0, 0
		}
	},
	{ 
		AM_TYPE_FLAT_MEM, /* Type */ 
		(1024 * 1024),    /* maxSizeBytes */
		7,    			  /* maxFunction  */	
		{
			(8 | 4 | 2 | 1),  /* Mem - Address Size */ 
			1,                /* Mem - Min Byte action */
			(1024 * 1024)     /* Mem - Max Byte action */
		}
	
	},
	{ 
		AM_TYPE_ARRAY,
		(1024 * 1024),
		8,
		{
			(8 | 4 | 2 | 1),  /* Array - Index Size Bytes */ 
			1,                /* Min Value Size - Bytes */
			1024,              /* Max Value Size - Bytes */
			TS_STAT_DT_FIXED_WIDTH
		}

	},
	{
		AM_TYPE_ASSOC_ARRAY,
		(1024 * 1024),
		7,
		{
			512,              /* Max - Key Size Bytes */ 
			1024 * 1024,      /* Max - Data Size Bytes */
			TS_ASSCA_KEY_FIXED_WIDTH | TS_ASSCA_KEY_VAR_WIDTH,
			TS_ASSCA_DATA_FIXED_WIDTH | TS_ASSCA_DATA_VAR_WIDTH
		}



	}

};
#endif





