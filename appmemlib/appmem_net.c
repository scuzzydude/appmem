
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "appmem_net.h"

void * am_net_resp_thread(void *p1)
{
	int count = 0;
	AMLIB_ENTRY_T *pEntry = p1;
	AM_ASSERT(pEntry);

	while(1)
	{
		printf("resp_thread =%d\n", count++);

	}


}



AM_RETURN am_net_init_entry(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr)
{
	AM_PACK_IDENTIFY id_t;
	AM_PACK_RESP_U idResp;
	AM_RETURN error = AM_RET_GOOD;
    UINT32 bytes_rcv;
    
	AM_DEBUGPRINT("am_net_init_entry IP ADDR =%08x\n", ipaddr);


	if(AM_RET_GOOD == am_net_establish_socket(pEntry, ipaddr, 0))
	{
		id_t.wrap.func_id = AM_PACK_FUNC_ID_BASEAPPMEM;
		id_t.wrap.packType = AM_PACK_TYPE_OPCODE_ONLY; 
		id_t.wrap.size = sizeof(AM_PACK_IDENTIFY);
		id_t.wrap.appTag = 7777; /* TODO - I'm not sure about other process on same server using the library */
		/* That's one reason to do it in the kernel as we'd esablish the socket */
		/* At driver load and it would be shared among processes */

		id_t.wrap.op = AM_OP_IDENTIFY;


		error = am_int_send_msg(pEntry->pTransport, &id_t, sizeof(AM_PACK_IDENTIFY));
		
		if(AM_RET_GOOD == error)
		{
			printf("Send Identify Packet Good\n");
			error = am_net_recv_msg(pEntry->pTransport, &idResp, sizeof(AM_PACK_RESP_U), &bytes_rcv);
			
			if(AM_RET_GOOD == error)
			{

				
				AM_DEBUGPRINT("Rcv Identify Packet Response\n");
				AM_DEBUGPRINT("packType            = 0x%04x\n", idResp.wrap.packType );
				AM_DEBUGPRINT("appTag              = 0x%04x\n", idResp.wrap.appTag );
				AM_DEBUGPRINT("func_id             = 0x%04x\n", idResp.wrap.func_id);
				AM_DEBUGPRINT("size                = 0x%04x\n", idResp.wrap.size);
				
				
				if(AM_FUNC_PACK_TYPE_FLAG_ERR & idResp.wrap.packType)
				{
					AM_DEBUGPRINT("error               = 0x%04x\n", idResp.error.status);
					error = idResp.error.status;
					if(0 == error)
					{
						/* Pack error flag but no status */
						error = AM_RET_INVALID_DEV;
					}
					else
					{


					}
				}
				else
				{
					AM_DEBUGPRINT("Identify GOOD \n");
					AM_DEBUGPRINT("amType = 0x%08x", idResp.identify.amType);
					AM_DEBUGPRINT("version = %08x , %d.%d.%d.%d\n", idResp.identify.amVersion.dwVersion,
						(int)idResp.identify.amVersion.version.major,
						(int)idResp.identify.amVersion.version.minor,
						(int)idResp.identify.amVersion.version.build,
						(int)idResp.identify.amVersion.version.bugfix);
					
					pEntry->pThread = am_thread_create(am_net_resp_thread, pEntry);


				}
			
			}

		}

	}
	else
	{
		error = AM_RET_SOCKET_ERR;
	}

	if(error != AM_RET_GOOD)
	{
		am_net_destroy_socket(pEntry);
	}


	return error;

}

AM_PACK_ALL_U gTxPacket;
AM_PACK_RESP_U gRxPacket;

typedef struct am_net_pack_transaction
{
	AM_PACK_ALL_U    *pTx;
	AM_PACK_RESP_U   *pRx;
	UINT16            tag;
	UINT32           done;        //TODO: semaphore
	UINT32           start_time;  //TODO: timeouts 

} AM_NET_PACK_TRANSACTION;

AM_NET_PACK_TRANSACTION gTransaction;

AM_NET_PACK_TRANSACTION *am_net_get_free_req(void)
{
	gTransaction.done = 0;
	gTransaction.tag = 1;
	gTransaction.start_time = 0xBA;
	gTransaction.pTx = &gTxPacket;
	gTransaction.pRx = NULL;
	gTransaction.pTx->wrap.appTag = gTransaction.tag;
	return &gTransaction;
}



UINT32 am_net_get_capabilites_count(AMLIB_ENTRY_T *pEntry)
{
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
    UINT32 bytes_rcv;
	UINT32 tx_size = sizeof(AM_PACK_GET_CAP_COUNT);

	pIop = am_net_get_free_req();
	pIop->pTx->cap_count.wrap.func_id;
	pIop->pTx->cap_count.wrap.func_id = AM_PACK_FUNC_ID_BASEAPPMEM;
	pIop->pTx->cap_count.wrap.packType = AM_PACK_TYPE_OPCODE_ONLY; 
	pIop->pTx->cap_count.wrap.size = tx_size;
	pIop->pTx->cap_count.wrap.op = AM_OP_GET_CAP_COUNT;

	error = am_int_send_msg(pEntry->pTransport, pIop->pTx, tx_size);

	while(1);

	return 0;
}


