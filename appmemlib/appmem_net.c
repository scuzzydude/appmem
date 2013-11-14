
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "appmem_net.h"


/* TODO: Globals are temp -- use a freelist */
AM_PACK_ALL_U gTxPacket;
AM_PACK_RESP_U gRxPacket;

typedef struct am_net_pack_transaction
{
	AM_PACK_ALL_U    *pTx;
	AM_PACK_RESP_U   *pRx;
	UINT16            tag;
	UINT32           done;        //TODO: semaphore
	UINT32           start_time;  //TODO: timeouts 
	UINT32           resp_bytes;

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


AM_PACK_RESP_U  * am_net_get_free_resp(AMLIB_ENTRY_T *pEntry, UINT32 *bytes_avail)
{
	*bytes_avail = sizeof(AM_PACK_RESP_U); 
	return &gRxPacket;

}

AM_RETURN am_net_signal_resp_recv(AM_PACK_RESP_U  *pRx, UINT32 bytes_rec)
{
	AM_NET_PACK_TRANSACTION *pIop = NULL;
	
	/* TODO -- lookup tag */
	if(1 == pRx->wrap.appTag)
	{
		pIop = &gTransaction;
	}

	if(NULL != pIop)
	{
		AM_DEBUGPRINT("signal_resp_recv tag=%d resp_bytes\n", pRx->wrap.appTag, bytes_rec);

		pIop->pRx = pRx;
		pIop->resp_bytes	= bytes_rec;
		pIop->done = 1;  /* TODO: Proper semaphor */
		return AM_RET_GOOD;
	}

	return AM_RET_TAG_NOT_FOUND;
}


void * am_net_resp_thread(void *p1)
{
	int count = 0;
	AMLIB_ENTRY_T *pEntry = (AMLIB_ENTRY_T *)p1;
	UINT32 bytes_avail;
	UINT32 bytes_rcv = 0;
	AM_PACK_RESP_U  *pRx;
	AM_RETURN error;

	AM_ASSERT(pEntry);


	while(1)
	{
		pRx = am_net_get_free_resp(pEntry, &bytes_avail);

		if(pRx)
		{
			error = am_net_recv_msg(pEntry->pTransport, pRx, bytes_avail, &bytes_rcv);

			if(AM_RET_GOOD == error)
			{
				AM_DEBUGPRINT("resp_thread RCV PACK bytes_rcv =%d\n", bytes_rcv);
				am_net_signal_resp_recv(pRx, bytes_rcv);
			}
			else
			{
				AM_DEBUGPRINT("resp_thread RCV error =%d\n", error);
			
			}
		
		
		}
	
		
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

AM_RETURN am_net_send_and_wait(AMLIB_ENTRY_T *pEntry, AM_NET_PACK_TRANSACTION *pIop, UINT32 tx_size, UINT32 timeOutMs)
{
	AM_RETURN error;

	error = am_int_send_msg(pEntry->pTransport, pIop->pTx, tx_size);

	if(AM_RET_GOOD == error)
	{
		while(0 == pIop->done)
		{
			AM_SLEEP(0);
		}
		
		if(pIop->done)
		{
			if(AM_FUNC_PACK_TYPE_FLAG_ERR & pIop->pRx->error.wrap.packType) 
			{
				error = pIop->pRx->error.status;
			}
			else
			{
				error = AM_RET_GOOD;
			}
		
		}

	}

	return error;

}
UINT32 am_net_get_capabilites_count(AMLIB_ENTRY_T *pEntry)
{
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size = sizeof(AM_PACK_GET_CAP_COUNT);
	UINT32 cap_count = 0;

	pIop = am_net_get_free_req();
	pIop->pTx->cap_count.wrap.func_id = AM_PACK_FUNC_ID_BASEAPPMEM;
	pIop->pTx->cap_count.wrap.packType = AM_PACK_TYPE_OPCODE_ONLY; 
	pIop->pTx->cap_count.wrap.size = tx_size;
	pIop->pTx->cap_count.wrap.op = AM_OP_GET_CAP_COUNT;

	error = am_net_send_and_wait(pEntry, pIop, tx_size, 5000);

	if(AM_RET_GOOD == error)
	{
		cap_count = pIop->pRx->cap_count.cap_count;

		AM_DEBUGPRINT("IOP DONE %p resp_bytes=%d cap_count=%d\n", pIop, pIop->resp_bytes, cap_count);
	}

	return cap_count;
}


AM_RETURN am_net_get_capabilities(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count)
{
	AM_NET_PACK_TRANSACTION *pIop;
	AM_RETURN error = AM_RET_GOOD;
 	UINT32 tx_size = sizeof(AM_PACK_FIXED_IN_VAR_OUT);
	int cap_bytes;

	pIop = am_net_get_free_req();

	pIop->pTx->fivo.wrap.func_id = AM_PACK_FUNC_ID_BASEAPPMEM;
	pIop->pTx->fivo.wrap.packType = AM_PACK_TYPE_FIVO; 
	pIop->pTx->fivo.wrap.size = tx_size;
	pIop->pTx->fivo.wrap.op = AM_OP_GET_CAP;
	pIop->pTx->fivo.l1 = sizeof(UINT32);
	*(UINT32 *) &pIop->pTx->fivo.data_in[0] = count;

	error = am_net_send_and_wait(pEntry, pIop, tx_size, 5000);

	if(AM_RET_GOOD == error)
	{
		cap_bytes = pIop->resp_bytes - sizeof(AM_PACK_WRAPPER_T);

		if((cap_bytes > 0) && (cap_bytes <= (int)(count * sizeof(AM_MEM_CAP_T))))
		{
			memcpy(pAmCaps, &pIop->pRx->align_resp.resp_bytes[0], cap_bytes);
		}
			
	}





	return AM_RET_GOOD;
}
