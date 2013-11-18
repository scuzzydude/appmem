#include <stdio.h>
#include <stdlib.h>
#include "appmemlib.h"
#include "am_test_os.h"
#include "appmem_net.h"
#include "am_stata.h"

#define SINGLE_THREAD_TARGET 0



#define AM_TARGET_MAX_FUNCTIONS          (256)
#define AM_TARGET_FUNCTION_ID_SIG        (0xAA00)
#define AM_TARGET_FUNCTION_ID_SIG_MASK   (0xFF00)
#define AM_TARGET_FUNCTION_ID_IDX_MASK   (0x00FF)
#define AM_TARG_GET_FUNC_IDX(_x) ( (AM_TARGET_FUNCTION_ID_SIG == (_x & AM_TARGET_FUNCTION_ID_SIG_MASK)) ? ((_x & AM_TARGET_FUNCTION_ID_IDX_MASK)) : 0xFF)

AM_MEM_FUNCTION_T * g_pfnFunctionTable[AM_TARGET_MAX_FUNCTIONS];




AM_PACK_QUEUE_T * am_init_pack_queue(UINT32 pack_count)
{
    AM_PACK_QUEUE_T *pQueue = NULL;
    AM_PACK_ALL_U *pRxBuf = NULL;
	AM_REC_CMD_T *pRxCmd = NULL;
	AM_PACK_RESP_U *pTxBuf = NULL;

	int error  = 0;
    UINT32 i;

	pRxCmd = (AM_REC_CMD_T *)AM_MALLOC(pack_count * sizeof(AM_REC_CMD_T));
	pRxBuf = (AM_PACK_ALL_U *)AM_MALLOC(pack_count * sizeof(AM_PACK_ALL_U));
	pTxBuf = (AM_PACK_RESP_U *)AM_MALLOC(pack_count * sizeof(AM_PACK_RESP_U));

    if((NULL != pRxBuf) && (NULL != pRxCmd) && (NULL != pTxBuf))
    {
        AM_DEBUGPRINT("pRxBuf(%d) = %p\n", pack_count * sizeof(AM_PACK_ALL_U), pRxBuf);
        AM_DEBUGPRINT("  pRxCmd(%d) = %p\n", sizeof(AM_REC_CMD_T) * pack_count, pRxCmd);
        
		memset(pRxCmd, 0, pack_count * sizeof(AM_REC_CMD_T));
		memset(pRxBuf, 0, pack_count * sizeof(AM_PACK_ALL_U));
		memset(pTxBuf, 0, pack_count * sizeof(AM_PACK_RESP_U));

		for(i = 0; i < pack_count; i++)
		{
			pRxCmd[i].pRxBuf = &pRxBuf[i];	
			pRxCmd[i].idx = i;
			pRxCmd[i].state = RX_CMD_STATE_FREE;
			pRxCmd[i].recv_len = 0;
			pRxCmd[i].pvClient = am_net_alloc_client();

			if(i < (pack_count - 1))
			{
				pRxCmd[i].next = &pRxCmd[i + 1];
			}
			else
			{
				/* TODO: Maybe this is circular */
				/* We can't use it as circular queue directly */
				/* as we will use only continuguous data buffer and they may be multiple packet unions */
				/* we always know head/tail pRxCmd[0] and pRxCmd[pack_count - 1] */
				pRxCmd[i].next = NULL;
			}
		
		
		}

		pQueue = (AM_PACK_QUEUE_T *)AM_MALLOC(sizeof(AM_PACK_QUEUE_T)); 
		
		if(pQueue)
        {
            pQueue->pRxBuf = pRxBuf;
			pQueue->pRxCmd = pRxCmd;
            pQueue->pTxBuf = pTxBuf;
			pQueue->size = pack_count;
            pQueue->rx_ci = 0;
            pQueue->rx_pi = 0;
            pQueue->tx_ci = 0;
            pQueue->tx_pi = 0;
			

        }
        else
        {
			error++;
		}


    }
	else
	{
		error++;
	}

	if(error)
	{
		if(pRxBuf)
		{
			AM_FREE(pRxBuf);
		}
		if(pTxBuf)
		{
			AM_FREE(pTxBuf);
		}
		if(pRxCmd)
		{
			AM_FREE(pRxCmd);
		}
		if(pQueue)
		{
			AM_FREE(pQueue);
		}
		pQueue = NULL;

	}
	
	
	return pQueue;
    
}


AM_RETURN am_update_pack_queue_rx_idx(AM_PACK_QUEUE_T *pQueue, AM_REC_CMD_T *pRxCmd)
{
	UINT32 inc_size = 0;
	AM_REC_CMD_T *pCmdNext;
	UINT32 i;
	AM_ASSERT(pQueue);
	AM_ASSERT(pRxCmd);
	AM_ASSERT((pRxCmd->idx == pQueue->rx_pi));

	if(pRxCmd->recv_len)
	{
		inc_size = (pRxCmd->recv_len  / sizeof(AM_PACK_ALL_U));

		if(pRxCmd->recv_len  % sizeof(AM_PACK_ALL_U))
		{
			inc_size++;
		}

		pQueue->rx_pi += inc_size;
		
		pRxCmd->buf_count = inc_size;

		
		
		if(pQueue->rx_pi > pQueue->size)
		{
			/* I'm assuming the the RECV packet function will error */
			/* If packet comes in bigger than max_bytes */
			/* So we shouldn't get here */
			/* Need to explore this later */
			/* We need to push back and either stall */
			/* Or wrap to front of the queue in this case */
			return AM_RET_Q_OVERFLOW;
		}
		
		pRxCmd->state = RX_CMD_STATE_ACTIVE;
		
		if(inc_size > 1)
		{
			pCmdNext = pRxCmd->next;
			i = 1;
			/* TODO: Mark the next series as inuse */
			while((NULL != pCmdNext) &&  (i < inc_size))
			{
					
				pCmdNext->state = RX_CMD_STATE_CONT_BUF_2 + (i - 1);
				i++;
			}
			
			AM_ASSERT(0);
		}
	
		
		
		
		
		if(pQueue->rx_pi == pQueue->size)
		{
			/* Normal Wrap Around when rcv_pack_size <= normal_pack_size */
			pQueue->rx_pi = 0;
		}
	
	
	}
	else
	{
		AM_ASSERT(0); //received a packet with 0 length? 
	}
	
	AM_DEBUGPRINT("pQueue->rx_pi = %d INC_SIZE=%d\n", pQueue->rx_pi, inc_size);
	return AM_RET_GOOD;
	
}


AM_PACK_RESP_U * am_get_pack_resp_buf(AM_PACK_QUEUE_T *pQueue, UINT32 *max_bytes)
{
	AM_ASSERT(pQueue);
	
	*max_bytes = sizeof(AM_PACK_RESP_U); /* TODO: Calculate number of continguous bytes left in the buffer */

	return &(pQueue->pTxBuf[pQueue->tx_pi]);

}

AM_REC_CMD_T * am_get_pack_queue_buf(AM_PACK_QUEUE_T *pQueue, UINT32 *max_bytes)
{

    AM_ASSERT(pQueue);
	
	*max_bytes = sizeof(AM_PACK_ALL_U); /* TODO: Calculate number of continguous bytes left in the buffer */

    return &(pQueue->pRxCmd[pQueue->rx_pi]);

}

AM_MEM_FUNCTION_T * am_validate_function_id(UINT16 func_id)
{
	UINT16 idx;

	AM_DEBUGPRINT("am_validate_function_id %04x\n",  func_id);

	if(AM_PACK_FUNC_ID_BASEAPPMEM == func_id)
	{
		AM_DEBUGPRINT("BASEAPPMEM\n");

		return g_pfnFunctionTable[0];
	}
	else
	{
		



		if(AM_TARGET_FUNCTION_ID_SIG == (func_id & AM_TARGET_FUNCTION_ID_SIG_MASK))
		{

			idx = (func_id & AM_TARGET_FUNCTION_ID_IDX_MASK);

			AM_ASSERT((idx < AM_TARGET_MAX_FUNCTIONS));

			return g_pfnFunctionTable[idx];
		}
		else
		{
			/* No signature */
			return NULL;
		}
	}

}



AM_RETURN am_targ_identify_resp(AM_MEM_FUNCTION_T *pFunc, AM_PACK_RESP_U *pResp, UINT32 *tx_bytes)
{
	AM_PACK_RESP_IDENTIFY *pIdResp = (AM_PACK_RESP_IDENTIFY *)pResp;
	
	AM_DEBUGPRINT("am_targ_identify_resp %p\n", pFunc);
	
	pIdResp->amType = 0xBBBB;
	pIdResp->amVersion.version.major = AM_VER_MAJOR;
	pIdResp->amVersion.version.minor = AM_VER_MINOR;
	pIdResp->amVersion.version.build = AM_VER_BUILD;
	pIdResp->amVersion.version.bugfix = AM_VER_BUGFIX;

	*tx_bytes = sizeof(AM_PACK_RESP_IDENTIFY); 
	



	return AM_RET_GOOD;
}

AM_RETURN am_targ_get_cap_count(AM_MEM_FUNCTION_T *pFunc, AM_PACK_RESP_U *pResp, UINT32 *tx_bytes)
{
	AM_PACK_RESP_CAP_COUNT *pCCResp = (AM_PACK_RESP_CAP_COUNT *)pResp;
	
    pCCResp->cap_count = sizeof(virtd_caps) / sizeof(AM_MEM_CAP_T);

	*tx_bytes = sizeof(AM_PACK_RESP_CAP_COUNT);
	
	return AM_RET_GOOD;

}

AM_RETURN am_targ_get_cap(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT64 l1, void *p2, UINT32 *ret_len)
{
	UINT32 count = (UINT32) *(UINT64 *)p1;
	UINT32 tx_bytes = count * sizeof(AM_MEM_CAP_T);

	AM_DEBUGPRINT("am_targ_get_cap count=%d tx_bytes=%d\n", count, tx_bytes);

	memcpy(p2, &virtd_caps,tx_bytes);
	*ret_len = tx_bytes;

	return AM_RET_GOOD;
}

AM_RETURN am_targ_release(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;
}

AM_RETURN am_targ_open(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	*ret_len = sizeof(AM_PACK_WRAPPER_T);
	AM_DEBUGPRINT("am_targ_open: %04x\n", pFunc->handle);

	return AM_RET_GOOD;
}

AM_RETURN am_targ_close(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT32 *ret_len)
{
	return AM_RET_GOOD;
}

AM_RETURN am_targ_create_stata_device(AM_MEM_FUNCTION_T *pFunc, AM_MEM_CAP_T *pCap)
{
    AM_RETURN error = 0;
    AM_FUNC_DATA_U *pVdF = NULL;
 
	AM_ASSERT(pCap);
	AM_ASSERT(pFunc);
	AM_ASSERT(pFunc->pfnOps);


    AM_DEBUGPRINT( "am_tar6g_create_stata_device: maxSize=%lld\n", pCap->maxSize );

    pVdF = (AM_FUNC_DATA_U *)AM_MALLOC(sizeof(AM_FUNC_DATA_U));
    
    if(pVdF)
    {
		pVdF->stata.idx_size = pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE];
		pVdF->stata.data_size = pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE];
		pVdF->stata.size = pVdF->stata.data_size * pCap->maxSize;
		pVdF->stata.array_size = pCap->maxSize;
		
		pVdF->stata.data = AM_MALLOC((size_t)pVdF->stata.size);
        
		pFunc->crResp.data_size = pVdF->stata.data_size;
		pFunc->crResp.idx_size = pVdF->stata.idx_size;


		if(NULL != pVdF->stata.data)
		{
		    pFunc->pfnOps[AM_OP_RELEASE_FUNC].op_only  = am_targ_release;
			pFunc->crResp.ops[AM_OP_RELEASE_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_RELEASE_FUNC;
			
			pFunc->pfnOps[AM_OP_OPEN_FUNC].op_only  = am_targ_open;
			pFunc->crResp.ops[AM_OP_OPEN_FUNC] =   (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_OPEN_FUNC;

			pFunc->pfnOps[AM_OP_CLOSE_FUNC].op_only  = am_targ_close;
			pFunc->crResp.ops[AM_OP_CLOSE_FUNC] = (AM_PACK_TYPE_OPCODE_ONLY << 16) | AM_OP_CLOSE_FUNC;

			//#define AM_OP_WRITE_ALIGN                    0x10
			//#define AM_OP_READ_ALIGN                     0x11
			
			pFunc->pfnOps[AM_OP_WRITE_ALIGN].align = am_stata_write_idx32;
			pFunc->pfnOps[AM_OP_READ_ALIGN].align = am_stata_read_idx32;
			 
			pFunc->crResp.acOps[ACOP_WRITE] = (AM_PACK_ALIGN << 16) | AM_OP_WRITE_ALIGN;
			pFunc->crResp.acOps[ACOP_READ] =  (AM_PACK_ALIGN << 16) | AM_OP_READ_ALIGN;

			
			pFunc->crResp.pack_DataOffset = pVdF->stata.idx_size;
			
			pFunc->pVdF = pVdF;
		
			
			sprintf(pFunc->crResp.am_name, "am_stata_%04x", pFunc->handle);
			AM_DEBUGPRINT("CREATE STATA = AM_NAME=%s\n", pFunc->crResp.am_name);		
			pFunc->crResp.devt = pFunc->handle;
		}
		else
		{
			error = AM_RET_ALLOC_ERR;
		}


	}
	else
	{
		error = AM_RET_ALLOC_ERR;
	}

	if(AM_RET_GOOD != error)
	{
		if(NULL != pVdF)
		{
			AM_FREE(pVdF);
		}
	}

	return error;
}
AM_RETURN am_targ_create_function(AM_MEM_FUNCTION_T *pFunc, void *p1, UINT64 l1, void *p2, UINT32 *ret_len)
{
	AM_RETURN error = AM_RET_GOOD;
	UINT32 tx_bytes = 0;
	AM_MEM_CAP_T *pCap = p1;
	UINT32 i;
	AM_MEM_FUNCTION_T * pDeviceFunc;

	AM_DEBUGPRINT("am_targ_create_funct amType=%d tx_bytes=%d\n", pCap->amType, tx_bytes);

	for(i = 0; i < AM_TARGET_MAX_FUNCTIONS; i++)
	{
		if(NULL == g_pfnFunctionTable[i])
		{
			pDeviceFunc = (AM_MEM_FUNCTION_T *)AM_MALLOC(sizeof(AM_MEM_FUNCTION_T));
			
			if(pDeviceFunc)
			{
				memset(pDeviceFunc, 0, sizeof(AM_MEM_FUNCTION_T));
				pDeviceFunc->handle = (AM_TARGET_FUNCTION_ID_SIG | i); 
				AM_DEBUGPRINT("function table slot found =%d Handle=%04x pFunc=%p\n", i, pDeviceFunc->handle, pDeviceFunc);
		
				pDeviceFunc->pfnOps = (AM_FN_U *) AM_MALLOC((sizeof(am_cmd_fn) * AM_OP_MAX_OPS));
				g_pfnFunctionTable[i] = pDeviceFunc;
				
				if(NULL != pDeviceFunc->pfnOps)
				{
					memset(pDeviceFunc->pfnOps, 0, (sizeof(am_cmd_fn) * AM_OP_MAX_OPS));
				}
				else
				{
					AM_FREE(pDeviceFunc);
					pFunc = NULL;
				}


				break;
			}
			else
			{
				return AM_RET_ALLOC_ERR;
			}

		}


	}

	if(NULL == pDeviceFunc)
	{
		return AM_RET_NO_DEVICE_SLOT;
	}

	switch(pCap->amType)
	{

		case AM_TYPE_ARRAY:
		{
			error = am_targ_create_stata_device(pDeviceFunc, pCap);
		}
		break;

		default:
		{
			error = AM_RET_INVALID_FUNC;
		}
		break;


	}

	if(error != AM_RET_GOOD)
	{
		if(pDeviceFunc)
		{
			i = AM_TARG_GET_FUNC_IDX(pDeviceFunc->handle);

			AM_DEBUGPRINT("Create function ERROR =%d DELETING IDX=%d\n", error, i);

			if(g_pfnFunctionTable[i] == pDeviceFunc)
			{
				g_pfnFunctionTable[i] = NULL;
			}

			AM_FREE(pDeviceFunc);
		}

	}
	else
	{
		/* copy the response */
		*ret_len = sizeof(APPMEM_RESP_CR_FUNC_T);
		memcpy(p2, &pDeviceFunc->crResp, *ret_len);
		AM_DEBUGPRINT("Create function cpy CrFunc bytes=%d\n", *ret_len);
		AM_DEBUGPRINT("---Device Created = AM_NAME=%s\n", pDeviceFunc->crResp.am_name);		
		AM_DEBUGPRINT("---Device Created = DEV_T=%08x\n", pDeviceFunc->crResp.devt);		

	
	}

	
	
	return error;
}





AM_RETURN am_targ_process_cmd(AM_REC_CMD_T *pRxCmd, AM_TARGET_T *pTarget)
{
	AM_RETURN error = 0;	
	AM_PACK_RESP_U *pResp;
	UINT32 max_resp_bytes;
	UINT32 tx_bytes = 0;
	AMLIB_ENTRY_T *pEntry;
	AM_MEM_FUNCTION_T *pFunc = NULL;
	AM_FN_U *opFn;
	UINT32 op;

	UINT32 i;

	AM_ASSERT(pTarget);

	pEntry = (AMLIB_ENTRY_T *)pTarget->pEntry;

	AM_ASSERT(pEntry);


	pResp = am_get_pack_resp_buf(pTarget->pPackQueue, &max_resp_bytes);

	if(NULL == pResp)
	{
		AM_ASSERT(0); /* in this simple model, each process (target) has it's own queue and this thread is sending the responses */
		/* No need to backpressure but perhaps we should always have an "emergency" resp frame we can send */
	}

	if(pRxCmd->recv_len < sizeof(AM_PACK_WRAPPER_T))
	{
		/* TODO deal with case where we get a stray packet */
		/* That doesn't have a wrapper */
		/* We should send a response to the port that send it */


		AM_ASSERT(0);
	}


	/* Copy the wrapper to the response */
	memcpy(&pResp->wrap, &pRxCmd->pRxBuf->wrap, sizeof(AM_PACK_WRAPPER_T));

	pResp->wrap.packType |= AM_FUNC_PACK_TYPE_FLAG_RESP;

	pFunc = am_validate_function_id(pRxCmd->pRxBuf->wrap.func_id);

	AM_DEBUGPRINT("process_cmd : pFunc =%p PackType=0x%04x\n", pFunc,  pRxCmd->pRxBuf->wrap.packType);


	if(NULL != pFunc)
	{

		opFn = pFunc->pfnOps;	
		op = pRxCmd->pRxBuf->wrap.op;

		AM_ASSERT(opFn);

		if(NULL != opFn[op].raw)
		{
			switch(pRxCmd->pRxBuf->wrap.packType)
			{

				case AM_PACK_ALIGN:
				{
					if(op & 1)
					{
						/* Read Align */	
						error = opFn[op].align(pFunc, &pRxCmd->pRxBuf->write_al.data_bytes[0], &pResp->align_resp.resp_bytes[0]);  
						tx_bytes = sizeof(AM_PACK_WRAPPER_T) + pFunc->crResp.data_size; /* If there is error, it will tx_bytes will be overridden below */

					}
					else
					{
						/* Write Align */
						error = opFn[op].align(pFunc, &pRxCmd->pRxBuf->write_al.data_bytes[0], &pRxCmd->pRxBuf->write_al.data_bytes[pFunc->pVdF->stata.idx_size]);  
						tx_bytes = sizeof(AM_PACK_WRAPPER_T); /* If there is error, it will tx_bytes will be overridden below */
					}
				}
				break;

			
				case AM_PACK_TYPE_OPCODE_ONLY:
				{
					error = opFn[op].op_only(pFunc, pResp, &tx_bytes);			
				}
				break;

				
				case AM_PACK_TYPE_FIVO:
				{
				
					error = opFn[op].fivo(pFunc, 
											&pRxCmd->pRxBuf->fivo.data_in[0], 
											pRxCmd->pRxBuf->fivo.l1,
											&pResp->align_resp.resp_bytes[0],
											&tx_bytes);

				
				}
				break;

				default:
				error = AM_RET_INVALID_PACK_OP;
				break;

			}
		}
		else
		{
			error = AM_RET_INVALID_OPCODE;
		}


	}
	else
	{
		error = AM_RET_INVALID_FUNC;
	}


	if(AM_RET_GOOD != error)
	{
		pResp->error.wrap.packType |= AM_FUNC_PACK_TYPE_FLAG_ERR;
		pResp->error.status = error;
		pResp->error.wrap.size = sizeof(AM_PACK_RESP_ERR);
		tx_bytes = sizeof(AM_PACK_RESP_ERR);
		AM_DEBUGPRINT("am_targ_process_cmd : pack_type=%04x size = %d\n", pResp->error.wrap.packType, pResp->error.wrap.size); 


	}
	else
	{
		pResp->wrap.size = tx_bytes;
	}



	AM_DEBUGPRINT("am_targ_process_cmd : tx_bytes=%d Error = %d\n", tx_bytes, error); 
#if 0
	for(i = 0; i < tx_bytes; i++)
	{
		AM_DEBUGPRINT("%02x ", pResp->raw[i]);
	}
	AM_DEBUGPRINT(" count =%d\n", tx_bytes);
#endif
	am_net_send_resp_msg(pEntry->pTransport, pResp, pRxCmd->pvClient, tx_bytes);



	return error;
}

UINT32 am_targ_check_cmd_queue(AM_TARGET_T *pTarget)
{
	UINT32 count = 0;
	AM_PACK_QUEUE_T *pQueue = NULL;
	AM_REC_CMD_T *pRxCmd;
	AMLIB_ENTRY_T *pEntry;

	AM_ASSERT(pTarget);

	pEntry = (AMLIB_ENTRY_T *)pTarget->pEntry;

	AM_ASSERT(pEntry);
	
	pQueue = pTarget->pPackQueue;
	
	AM_ASSERT(pQueue);

	while(pQueue->rx_ci != pQueue->rx_pi)
	{
			pRxCmd = &pQueue->pRxCmd[pQueue->rx_ci];


			AM_DEBUGPRINT("**** CONSUME THREAD (%d)***\n", count);
            AM_DEBUGPRINT("message RECIEVED LEN = %d (0x%08x)\n", pRxCmd->recv_len, pRxCmd->recv_len);
            AM_DEBUGPRINT("packType =     %04x\n", pRxCmd->pRxBuf->wrap.packType);
            AM_DEBUGPRINT("appTag   =     %04x\n", pRxCmd->pRxBuf->wrap.appTag);
            AM_DEBUGPRINT("size     =     %04x\n", pRxCmd->pRxBuf->wrap.size);
            AM_DEBUGPRINT("op       = %08x\n",     pRxCmd->pRxBuf->wrap.op);

			if(AM_RET_GOOD == am_targ_process_cmd(pRxCmd, pTarget))
			{

			}
			else
			{
				/* TODO -- Some error, stall, backpressure etc... some action */
			}
	
			pQueue->rx_ci += pRxCmd->buf_count;
			pQueue->rx_ci = (pQueue->rx_ci % pQueue->size); 
			
			count++;
	}

	return count;

}




void * am_targ_recv_thread(void *p1)
{
    int count = 0;
    AMLIB_ENTRY_T *pEntry = (AMLIB_ENTRY_T *)p1;
    AM_TARGET_T *pTarget = pEntry->pTarget;
    AM_PACK_QUEUE_T *pQueue;
    void *pTransport;
    UINT32 max_len_bytes;
    AM_REC_CMD_T *pRxCmd;
	AM_RETURN error;

    pTransport = pEntry->pTransport;
    pQueue = pTarget->pPackQueue;

    AM_ASSERT(pQueue);
    
    while(1)
    {
        AM_DEBUGPRINT("Thread count =%d\n", count);
        count++;

        pRxCmd = am_get_pack_queue_buf(pQueue, &max_len_bytes);

        AM_DEBUGPRINT("pPack(%d) = %p\n", max_len_bytes, pRxCmd);
        
        if(AM_RET_GOOD ==  am_net_recv_unsol_msg(pTransport, pRxCmd->pRxBuf, max_len_bytes, &pRxCmd->recv_len, pRxCmd->pvClient))
//        if(AM_RET_GOOD == am_net_recv_msg(pTransport, pRxCmd->pRxBuf, max_len_bytes, &pRxCmd->recv_len))
        {
            AM_DEBUGPRINT("message RECIEVED LEN = %d (0x%08x)\n", pRxCmd->recv_len, pRxCmd->recv_len);
            AM_DEBUGPRINT("packType = %08x\n", pRxCmd->pRxBuf->wrap.packType);
            AM_DEBUGPRINT("appTag   =     %04x\n", pRxCmd->pRxBuf->wrap.appTag);
            AM_DEBUGPRINT("size     =     %04x\n", pRxCmd->pRxBuf->wrap.size);

			error = am_update_pack_queue_rx_idx(pQueue, pRxCmd);

			if(AM_RET_GOOD == error)
			{
				AM_DEBUGPRINT("Queue RX_IDX updated\n");
			}

#if SINGLE_THREAD_TARGET

			if(AM_RET_GOOD == am_targ_process_cmd(pRxCmd, pTarget))
			{

			}
			else
			{
				/* TODO -- Some error, stall, backpressure etc... some action */
			}
	
			pQueue->rx_ci += pRxCmd->buf_count;
			pQueue->rx_ci = (pQueue->rx_ci % pQueue->size); 
#endif


        }




//        AM_SLEEP(1);


    }

    return NULL;
}





AM_RETURN am_targ_recv_thread_create(AMLIB_ENTRY_T	*pEntry)
{
    void *pThread = NULL;
    AM_TARGET_T *pTarget = (AM_TARGET_T *)pEntry->pTarget;

    pThread = am_thread_create(am_targ_recv_thread, pEntry);
    AM_ASSERT(pEntry->pTarget);
    
    if(pThread)
    {
        pTarget->pThread = pThread;

        AM_DEBUGPRINT("Targ Rev Thread Good %p\n", pThread);
        return AM_RET_GOOD;
        
    }
    else
    {
        AM_DEBUGPRINT("Targ Rev Thread Failed %p\n");
        return AM_RET_THREAD_ERR;
        
    }

}

AM_RETURN am_targ_init_base_function(AM_TARGET_T *pTarget)
{
	AM_MEM_FUNCTION_T *pBaseFunc = NULL;

	AM_ASSERT(pTarget);

	pBaseFunc = (AM_MEM_FUNCTION_T *)AM_MALLOC(sizeof(AM_MEM_FUNCTION_T));

	if(pBaseFunc)
	{
		memset(pBaseFunc, 0, sizeof(AM_MEM_FUNCTION_T));
	
		pBaseFunc->pfnOps = (AM_FN_U *) AM_MALLOC((sizeof(am_cmd_fn) * AM_OP_MAX_OPS));

		if(pBaseFunc->pfnOps)
		{
			memset(pBaseFunc->pfnOps, 0, (sizeof(am_cmd_fn) * AM_OP_MAX_OPS));
		
			pBaseFunc->pfnOps[AM_OP_IDENTIFY].op_only = (am_fn_op_only)am_targ_identify_resp;
			pBaseFunc->pfnOps[AM_OP_GET_CAP_COUNT].op_only = (am_fn_op_only) am_targ_get_cap_count;
			pBaseFunc->pfnOps[AM_OP_GET_CAP].fivo = (am_fivo) am_targ_get_cap;
			pBaseFunc->pfnOps[AM_OP_CREATE_FUNC].fivo = (am_fivo)am_targ_create_function;
		}
		else
		{
			AM_FREE(pBaseFunc);
			return AM_RET_ALLOC_ERR;
		}





		
		
		pTarget->pBaseFunc = pBaseFunc;
	
		
	
	}
	else
	{
		return AM_RET_ALLOC_ERR;
	}

	return AM_RET_GOOD;


}




AM_RETURN am_targ_start_target(void)
{
    AM_RETURN error;
    AMLIB_ENTRY_T	targEntry;
    AM_TARGET_T *pTarget = NULL;
	int i;

    memset(&targEntry, 0, sizeof(AMLIB_ENTRY_T));


    pTarget = (AM_TARGET_T *)AM_MALLOC(sizeof(AM_TARGET_T));

    if(pTarget == NULL)
    {
        return AM_RET_ALLOC_ERR;
    }

    memset(pTarget, 0, sizeof(AM_TARGET_T));
    
	


    targEntry.pTarget = pTarget;
	pTarget->pEntry = &targEntry;





	if(AM_RET_GOOD != am_targ_init_base_function(pTarget))
	{
		AM_FREE(pTarget);
		return AM_RET_ALLOC_ERR;
		
	}

	for(i = 0; i < AM_TARGET_MAX_FUNCTIONS; i++)
	{
		g_pfnFunctionTable[i] = NULL;
	}
	g_pfnFunctionTable[0] = pTarget->pBaseFunc;





    error = am_net_establish_socket(&targEntry, 0, 4950);
    
    
    if(AM_RET_GOOD == error)
    {
        AM_DEBUGPRINT("Socket Established %p\n", targEntry.pTransport);

        pTarget->pPackQueue = am_init_pack_queue(1024); /* 8K - packets are multiple of 8 bytes for Appmem */    
            

        if((pTarget->pPackQueue) && (AM_RET_GOOD == am_targ_recv_thread_create(&targEntry)))
        {
            AM_DEBUGPRINT("Start Target Loop Waiting on Threads...\n");
            while(1)
            {
#if SINGLE_THREAD_TARGET
				AM_SLEEP(10);
#else
				am_targ_check_cmd_queue(pTarget);
#endif
			}
            
        }
        else
        {
            AM_DEBUGPRINT("Main Loop Error...\n");
            
        }


    }

    
    return error;
}

int main(int argc, char **argv)
{
    AM_RETURN error;
    
    AM_DEBUGPRINT("am_targ - Appmem Target\n");

    error = am_targ_start_target();
    
	/* Check environment */
	AM_ASSERT((sizeof(AM_PACK_ALL_U) == MAX_BASIC_PACK_UNION_SIZE));
	AM_ASSERT((sizeof(UINT64) == 8));	
	
	
	
	
	
	
	if(AM_RET_GOOD != error)
    {
        AM_DEBUGPRINT("Start Target error = %d\n", error);

    }
    return 0;
}
