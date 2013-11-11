#ifndef _APPMEM_NET_H
#define _APPMEM_NET_H

/* TODO - determine port number range */
#define DEFAULT_AM_NET_PORT 4950
/* common */
AM_RETURN am_net_init_entry(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr);


/* Os Specific */
AM_RETURN am_net_establish_socket(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr);
AM_RETURN am_int_send_msg(void *pTransport, void *pMsg, UINT32 len);
AM_RETURN am_int_recv_msg(void *pTransport, void *pMsg, UINT32 len);



#endif




