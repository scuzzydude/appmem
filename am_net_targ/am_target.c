#include <stdio.h>
#include <stdlib.h>
#include "appmemlib.h"
#include "am_test_os.h"
#include "appmem_net.h"


AM_PACK_QUEUE_T * am_init_pack_queue(UINT32 pack_count)
{
    AM_PACK_QUEUE_T *pQueue = NULL;
    AM_PACK_ALL_U *pRxBuf = NULL;
	AM_REC_CMD_T *pRxCmd = NULL;
	int error  = 0;
    UINT32 i;

	pRxCmd = (AM_REC_CMD_T *)AM_MALLOC(pack_count * sizeof(AM_REC_CMD_T));
	pRxBuf = (AM_PACK_ALL_U *)AM_MALLOC(pack_count * sizeof(AM_PACK_ALL_U));

    if((NULL != pRxBuf) && (NULL != pRxCmd))
    {
        printf("pRxBuf(%d) = %p\n", pack_count * sizeof(AM_PACK_ALL_U), pRxBuf);
        printf("  pRxCmd(%d) = %p\n", sizeof(AM_REC_CMD_T) * pack_count, pRxCmd);
        
		memset(pRxCmd, 0, pack_count * sizeof(AM_REC_CMD_T));
		memset(pRxBuf, 0, pack_count * sizeof(AM_PACK_ALL_U));



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
	
	printf("pQueue->rx_pi = %d INC_SIZE=%d\n", pQueue->rx_pi, inc_size);
	return AM_RET_GOOD;
	
}


AM_REC_CMD_T * am_get_pack_queue_buf(AM_PACK_QUEUE_T *pQueue, UINT32 *max_bytes)
{

    AM_ASSERT(pQueue);
	
	*max_bytes = sizeof(AM_PACK_ALL_U); /* TODO: Calculate number of continguous bytes left in the buffer */

    return &(pQueue->pRxCmd[pQueue->rx_pi]);

}

AM_RETURN am_validate_function_id(UINT16 packOp, UINT16 func_id)
{
	if(0)
	{
		//TODO: Lookup index of the function device 
	}
	if(AM_PACK_FUNC_ID_BASEAPPMEM == func_id)
	{
		return AM_RET_GOOD;
	}
	else
	{
		/* Can send identify to any device number (discovery) */
		if(AM_FUNC_OPCODE_IDENTIFY == packOp)
		{
			return AM_RET_GOOD;
		}
	
	}



	return AM_RET_INVALID_FUNC;
}

AM_RETURN am_targ_process_cmd(AM_REC_CMD_T *pRxCmd, AM_TARGET_T *pTarget)
{
	AM_RETURN error;	

	error = am_validate_function_id(pRxCmd->pRxBuf->wrap.func_id, pRxCmd->pRxBuf->wrap.func_id);
	
	if(AM_RET_GOOD == error)
	{

		switch(pRxCmd->pRxBuf->wrap.func_id)
		{


			default:
				error = AM_RET_INVALID_PACK_OP;
				break;

		}
	
	
	
	}
	else
	{
		error = AM_RET_INVALID_FUNC;
	}

	return error;
}

UINT32 am_targ_check_cmd_queue(AM_TARGET_T *pTarget)
{
	UINT32 count = 0;
	AM_PACK_QUEUE_T *pQueue = NULL;
	AM_REC_CMD_T *pRxCmd;

	AM_ASSERT(pTarget);
	
	pQueue = pTarget->pPackQueue;
	
	AM_ASSERT(pQueue);

	while(pQueue->rx_ci != pQueue->rx_pi)
	{
			pRxCmd = &pQueue->pRxCmd[pQueue->rx_ci];


			printf("**** CONSUME THREAD ***\n");
            printf("message RECIEVED LEN = %d (0x%08x)\n", pRxCmd->recv_len, pRxCmd->recv_len);
            printf("packType = %08x\n", pRxCmd->pRxBuf->wrap.packType);
            printf("appTag   =     %04x\n", pRxCmd->pRxBuf->wrap.appTag);
            printf("size     =     %04x\n", pRxCmd->pRxBuf->wrap.size);

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
        printf("Thread count =%d\n", count);
        count++;

        pRxCmd = am_get_pack_queue_buf(pQueue, &max_len_bytes);

        printf("pPack(%d) = %p\n", max_len_bytes, pRxCmd);
        
        if(AM_RET_GOOD ==  am_net_recv_unsol_msg(pTransport, pRxCmd->pRxBuf, max_len_bytes, &pRxCmd->recv_len, pRxCmd->pvClient))
//        if(AM_RET_GOOD == am_net_recv_msg(pTransport, pRxCmd->pRxBuf, max_len_bytes, &pRxCmd->recv_len))
        {
            printf("message RECIEVED LEN = %d (0x%08x)\n", pRxCmd->recv_len, pRxCmd->recv_len);
            printf("packType = %08x\n", pRxCmd->pRxBuf->wrap.packType);
            printf("appTag   =     %04x\n", pRxCmd->pRxBuf->wrap.appTag);
            printf("size     =     %04x\n", pRxCmd->pRxBuf->wrap.size);

			error = am_update_pack_queue_rx_idx(pQueue, pRxCmd);

			if(AM_RET_GOOD == error)
			{
				printf("Queue RX_IDX updated\n");
			}


        }




        AM_SLEEP(1);


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

        printf("Targ Rev Thread Good %p\n", pThread);
        return AM_RET_GOOD;
        
    }
    else
    {
        printf("Targ Rev Thread Failed %p\n");
        return AM_RET_THREAD_ERR;
        
    }

}



AM_RETURN am_targ_start_target(void)
{
    AM_RETURN error;
    AMLIB_ENTRY_T	targEntry;
    AM_TARGET_T *pTarget = NULL;

    memset(&targEntry, 0, sizeof(AMLIB_ENTRY_T));


    pTarget = (AM_TARGET_T *)AM_MALLOC(sizeof(AM_TARGET_T));

    if(pTarget == NULL)
    {
        return AM_RET_ALLOC_ERR;
    }

    memset(pTarget, 0, sizeof(AM_TARGET_T));
    
    targEntry.pTarget = pTarget;
    pTarget->pEntry = &targEntry;


    error = am_net_establish_socket(&targEntry, 0, 4950);
    
    
    if(AM_RET_GOOD == error)
    {
        printf("Socket Established %p\n", targEntry.pTransport);

        pTarget->pPackQueue = am_init_pack_queue(1024); /* 8K - packets are multiple of 8 bytes for Appmem */    
            

        if((pTarget->pPackQueue) && (AM_RET_GOOD == am_targ_recv_thread_create(&targEntry)))
        {
            printf("Start Target Loop Waiting on Threads...\n");
            while(1)
            {
				am_targ_check_cmd_queue(pTarget);
			}
            
        }
        else
        {
            printf("Main Loop Error...\n");
            
        }


    }

    
    return error;
}

int main(int argc, char **argv)
{
    AM_RETURN error;
    
    printf("am_targ - Appmem Target\n");

    error = am_targ_start_target();
    
	/* Check environment */
	AM_ASSERT((sizeof(AM_PACK_ALL_U) == MAX_BASIC_PACK_UNION_SIZE));
	AM_ASSERT((sizeof(UINT64) == 8));	
	
	
	
	
	
	
	if(AM_RET_GOOD != error)
    {
        printf("Start Target error = %d\n", error);

    }
    return 0;
}
