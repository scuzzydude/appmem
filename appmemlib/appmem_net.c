
#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "appmem_net.h"

AM_RETURN am_net_init_entry(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr)
{
	AM_PACK_IDENTIFY id_t;
	UINT8 tempbuf[100];
	am_net_establish_socket(pEntry, ipaddr);

	id_t.wrap.packType = AM_PACK_TYPE_OPCODE_ONLY; 
	id_t.wrap.size = sizeof(AM_PACK_IDENTIFY);
	id_t.wrap.appTag = 7777; /* TODO - I'm not sure about other process on same server using the library */
	                         /* That's one reason to do it in the kernel as we'd esablish the socket */
	                         /* At driver load and it would be shared among processes */

	id_t.op = AM_FUNC_OPCODE_IDENTIFY;

	if(AM_RET_GOOD == am_int_send_msg(pEntry->pTransport, &id_t, sizeof(AM_PACK_IDENTIFY)))
	{
		printf("Send Identify Packet Good\n");
		if(AM_RET_GOOD == am_int_recv_msg(pEntry->pTransport, &tempbuf[0], 100))
		{
			printf("Rcv Identify Packet Response\n");
		}

	}

	
	return AM_RET_GOOD;

}

