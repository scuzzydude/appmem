#ifndef _APPMEM_NET_H
#define _APPMEM_NET_H

/* TODO - determine port number range */
#define DEFAULT_AM_NET_PORT 4950
/* common */
AM_RETURN am_net_init_entry(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr);



/* Os Specific */
AM_RETURN am_net_establish_socket(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr, UINT16 port);
AM_RETURN am_int_send_msg(void *pTransport, void *pMsg, UINT32 len);
AM_RETURN am_net_recv_msg(void *pTransport, void *pMsg, UINT32 len, UINT32 *rcv_bytes);
AM_RETURN am_net_destroy_socket(AMLIB_ENTRY_T *pEntry);









/* TODO: Thread/Target specific stuff needs to be elsewhere, as targets may not be NET */
/* And non NET targets (or other appmem code) may use threads */
/* Keep it together for now as it is OS specific like the NET calls */
typedef struct _am_target
{
    void              *pThread;
    void              *pEntry;
    AM_PACK_QUEUE_T   *pPackQueue;
    
} AM_TARGET_T;

typedef void * (*am_fn_thread)(void * p1);
void * am_thread_create(am_fn_thread thread_fn, void *arg);




#endif




