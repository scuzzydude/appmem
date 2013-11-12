#include <stdio.h>
#include <stdlib.h>
#include "appmemlib.h"
#include "am_test_os.h"
#include "appmem_net.h"


AM_PACK_QUEUE_T * am_init_pack_queue(UINT32 qword_count)
{
    AM_PACK_QUEUE_T *pQueue = NULL;
    UINT64 *pQwbuf;

    pQwbuf = (UINT64 *)AM_MALLOC(qword_count * sizeof(UINT64));

    if(pQwbuf)
    {
        printf("pQwbuf(%d) = %p\n", (qword_count * sizeof(UINT64), pQwbuf));
        
        pQueue = AM_MALLOC(sizeof(AM_PACK_QUEUE_T)); 

        if(pQueue)
        {
            pQueue->pQwbuf = pQwbuf;
            pQueue->size = qword_count;
            pQueue->ci = 0;
            pQueue->pi = 0;


        }
        else
        {
            AM_FREE(pQwbuf);
        }


    }

    return pQueue;
    
}


UINT64 * am_get_pack_queue_buf(AM_PACK_QUEUE_T *pQueue, UINT32 min_qwords)
{

    AM_ASSERT(pQueue);

    return &(pQueue->pQwbuf[pQueue->pi]);


}








void * am_targ_recv_thread(void *p1)
{
    int count = 0;
    AMLIB_ENTRY_T *pEntry = (AMLIB_ENTRY_T *)p1;
    AM_TARGET_T *pTarget = pEntry->pTarget;
    AM_PACK_QUEUE_T *pQueue;
    void *pTransport;
    UINT32 rcv_bytes;
    const int max_len_bytes = (512);
    const int max_len_qwords = max_len_bytes / 8; 
    AM_PACK_ALL_U *pPack;
    
    pTransport = pEntry->pTransport;
    pQueue = pTarget->pPackQueue;

    AM_ASSERT(pQueue);
    
    while(1)
    {
        printf("Thread count =%d\n", count);
        count++;

        pPack = (AM_PACK_ALL_U *)am_get_pack_queue_buf(pQueue, max_len_qwords);

        printf("pPack(%d) = %p\n", max_len_bytes, pPack);
        
        
        if(AM_RET_GOOD == am_net_recv_msg(pTransport, pPack, max_len_bytes, &rcv_bytes))
        {
            printf("message RECIEVED LEN = %d (0x%08x)\n", rcv_bytes, rcv_bytes);
            printf("packType = %08x\n", pPack->wrap.packType);
            printf("appTag   =     %04x\n", pPack->wrap.appTag);
            printf("size     =     %04x\n", pPack->wrap.size);


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
                AM_SLEEP(1);
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
    
    if(AM_RET_GOOD != error)
    {
        printf("Start Target error = %d\n", error);

    }
    return 0;
}
