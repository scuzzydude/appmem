#ifndef _APPMEM_PACK_H
#define _APPMEM_PACK_H
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "appmem_net.h"
#include "am_stata.h"



typedef struct am_net_pack_transaction
{
	AM_PACK_ALL_U    *pTx;
	AM_PACK_RESP_U   *pRx;
	UINT16            tag;
	UINT32           done;        //TODO: semaphore
	UINT32           start_time;  //TODO: timeouts 
	UINT32           resp_bytes;

} AM_NET_PACK_TRANSACTION;

AM_RETURN am_pack_read_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2);
AM_RETURN am_pack_read32_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2);
AM_RETURN am_pack_write_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2);
AM_RETURN am_pack_write32_align(AM_MEM_FUNCTION_T *pFunc, void * p1, void *p2);
AM_RETURN am_pack_sort(AM_MEM_FUNCTION_T *pFunc, void * p1, UINT64 l1);


AM_RETURN am_pack_op_only(AM_MEM_FUNCTION_T *pFunc, UINT8 op, void *pResp, UINT32 resp_len);
AM_RETURN am_net_close(void * p1);
AM_RETURN am_net_open(void * p1);

#endif

