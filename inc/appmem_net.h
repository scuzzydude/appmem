#ifndef _APPMEM_NET_H
#define _APPMEM_NET_H

/* TODO - determine port number range */
#define DEFAULT_AM_NET_PORT 4950
/* common */
AM_RETURN am_net_init_entry(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr);
UINT32 am_net_get_capabilites_count(AMLIB_ENTRY_T *pEntry);
AM_RETURN am_net_get_capabilities(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count);
AM_RETURN am_net_create_function(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc);
AM_RETURN am_net_entry_close(AMLIB_ENTRY_T *pEntry);





/* Os Specific */
AM_RETURN am_net_establish_socket(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr, UINT16 port);
AM_RETURN am_int_send_msg(void *pTransport, void *pMsg, UINT32 len);
AM_RETURN am_net_recv_msg(void *pTransport, void *pMsg, UINT32 len, UINT32 *rcv_bytes);
AM_RETURN am_net_destroy_socket(AMLIB_ENTRY_T *pEntry);
void *    am_net_alloc_client(void);
AM_RETURN am_net_recv_unsol_msg(void *pTransport, void *pMsg, UINT32 len, UINT32 *rcv_bytes, void *pvClient);
AM_RETURN am_net_send_resp_msg(void *pTransport, void *pMsg, void *pvClient, UINT32 len);









/* TODO: Thread/Target specific stuff needs to be elsewhere, as targets may not be NET */
/* And non NET targets (or other appmem code) may use threads */
/* Keep it together for now as it is OS specific like the NET calls */
typedef struct _am_target
{
    void              *pThread;
    void              *pEntry;
    AM_PACK_QUEUE_T   *pPackQueue;
    AM_MEM_FUNCTION_T *pBaseFunc;

} AM_TARGET_T;

typedef void * (*am_fn_thread)(void * p1);
void * am_thread_create(am_fn_thread thread_fn, void *arg);
AM_RETURN am_thread_destroy(void *pvThread);




#endif




