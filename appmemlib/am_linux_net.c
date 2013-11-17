#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "appmem_net.h"


typedef struct _am_udp_socket
{
    int                  socket_handle;
    struct sockaddr_in   local_addr;
    struct sockaddr_in   remote_addr;
    struct timeval       rcv_timeout_tv;
    struct timeval       send_timeout_tv;
    

} AM_UPD_SOCKET_T;



AM_RETURN am_net_establish_socket(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr, UINT16 port)
{
    AM_UPD_SOCKET_T *pSocket = NULL;
    int error;
    int socket_handle;

    socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
    

    if(socket_handle < 0)
    {
         printf("Can not open socket %d", socket_handle);
		 return AM_RET_SOCKET_ERR;
    }

    pSocket = AM_MALLOC(sizeof(AM_UPD_SOCKET_T));

    if(NULL == pSocket)
    {
        printf("Malloc error\n");
    	return AM_RET_ALLOC_ERR;
	}

	memset(pSocket, 0, sizeof(AM_UPD_SOCKET_T));

    pSocket->rcv_timeout_tv.tv_sec = 10;

    if(setsockopt(socket_handle, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&pSocket->rcv_timeout_tv, sizeof(struct timeval)))
    {
        printf("Socket Set Rcv Timeout Value = %d\n", pSocket->rcv_timeout_tv.tv_sec);
        AM_FREE(pSocket);
    	return AM_RET_SOCKET_ERR;
	
    }


    pSocket->send_timeout_tv.tv_sec = 10;


    


    pSocket->local_addr.sin_family = AF_INET;
//    pSocket->local_addr.sin_port = htons(DEFAULT_AM_NET_PORT);
    if(port == 0)
    {
        pSocket->local_addr.sin_port = htons(1117); /* if the client is local host it needs a different port */
    }
    else
    {
        pSocket->local_addr.sin_port = htons(port); /* if the client is local host it needs a different port */
    }
    
    printf("Socket Port =%d\n", port);

    pSocket->local_addr.sin_addr.s_addr = INADDR_ANY;
 
    error = bind(socket_handle, (struct sockaddr *)&pSocket->local_addr, sizeof(struct sockaddr)); 

    if(error < 0)
    {
        printf("Bind Error\n");
        AM_FREE(pSocket);
		return AM_RET_SOCKET_ERR;
        
    }


    if(0 != ipaddr)
    {
        pSocket->remote_addr.sin_family = AF_INET;
        pSocket->remote_addr.sin_port = htons(DEFAULT_AM_NET_PORT);
        pSocket->remote_addr.sin_addr.s_addr = htonl(ipaddr);
    }

    pSocket->socket_handle = socket_handle;
    pEntry->pTransport = pSocket;
 
    return AM_RET_GOOD;



}

AM_RETURN am_int_send_msg(void *pTransport, void *pMsg, UINT32 len)
{

    AM_UPD_SOCKET_T       *pSocket = (AM_UPD_SOCKET_T *)pTransport;
	int bytes_sent;
	AM_ASSERT(pSocket);
   
    bytes_sent = sendto(pSocket->socket_handle, pMsg, len, 0, (struct sockaddr *)&pSocket->remote_addr, sizeof(struct sockaddr));

    if(bytes_sent < 0)
    {
	    return AM_RET_IO_ERR;
    }
    else if(bytes_sent != len)
    {
        return AM_RET_IO_UNDERRUN;
    }

    return AM_RET_GOOD;

}




AM_RETURN am_net_send_resp_msg(void *pTransport, void *pMsg, void *pvClient, UINT32 len)
{
	AM_UPD_SOCKET_T       *pSocket = (AM_UPD_SOCKET_T *)pTransport;
	struct sockaddr *pClient = (struct sockaddr *)pvClient;
	int client_len = sizeof(struct sockaddr_in);
	int error;

	AM_ASSERT(pSocket);

	error = sendto(pSocket->socket_handle, pMsg, len, 0, (struct sockaddr *)pClient, client_len);

    if(error < 0)
	{
	
		printf("Error transmitting data =%d\n", error );
		
		
		return AM_RET_IO_ERR;
	}

	return AM_RET_GOOD;
}


AM_RETURN am_net_recv_msg(void *pTransport, void *pMsg, UINT32 len, UINT32 *rcv_bytes)
{
    AM_UPD_SOCKET_T       *pSocket = (AM_UPD_SOCKET_T *)pTransport;
    int bytes_rcv;
    //socklen_t addr_len = sizeof(struct sockaddr);
    struct sockaddr_in their_addr;
    int addr_len;
    addr_len = sizeof(struct sockaddr);


	AM_ASSERT(pSocket);
    
//    printf("recvfrom ....\n");
    
//    bytes_rcv = recvfrom(pSocket->socket_handle, pMsg, len, 0, (struct sockaddr *)&pSocket->remote_addr, &addr_len );

    bytes_rcv = recvfrom(pSocket->socket_handle, pMsg, len, 0, (struct sockaddr *)&their_addr, &addr_len );

    if(bytes_rcv < 0)
    {

        printf("Rcv Error = %d\n", bytes_rcv);
        printf( "Rcv Error(%d): %s\n", errno, strerror( errno ) );
 
        return AM_RET_IO_ERR;
    }
    *rcv_bytes = bytes_rcv;

    return AM_RET_GOOD;
    
}

AM_RETURN am_net_recv_unsol_msg(void *pTransport, void *pMsg, UINT32 len, UINT32 *rcv_bytes, void *pvClient)
{
	AM_UPD_SOCKET_T        *pSocket = (AM_UPD_SOCKET_T *)pTransport;
	int error;
	struct sockaddr *pClient = pvClient;
	int client_len = sizeof(struct sockaddr_in);

	AM_ASSERT(pSocket);
	AM_ASSERT(pClient);
	error = recvfrom(pSocket->socket_handle, pMsg, len, 0, (struct sockaddr *)pClient, &client_len);
	
	if(error < 0)
	{
		printf("Error receiving data =%d.\n", error);
		return AM_RET_IO_ERR;
	}
	else
	{
		*rcv_bytes = error;
	}
	return AM_RET_GOOD;


}





AM_RETURN am_net_destroy_socket(AMLIB_ENTRY_T *pEntry)
{
    AM_UPD_SOCKET_T       *pSocket;
    AM_RETURN error = AM_RET_GOOD;

    AM_ASSERT(pEntry);
    pSocket = pEntry->pTransport;

    if(pSocket)
    {

        if(close(pSocket->socket_handle) != 0)
        {
       		error = AM_RET_SOCKET_ERR;
        }

        AM_FREE(pSocket);

    
    }

	return error;

}



/***************************************************************************************/
/* Thread Specific stuff - move to another file later                                 **/
/***************************************************************************************/


typedef struct _am_thread
{
    pthread_t        thread_id;
    pthread_attr_t   attr;
    am_fn_thread     thread_fn;
    void             *thread_arg;

} AM_THREAD_T;


void * am_thread_create(am_fn_thread thread_fn, void *arg)
{

    AM_THREAD_T *pThread = NULL;
  
    int s;
    void *res;
    printf("am_thread_create()\n");


    pThread = AM_MALLOC(sizeof(AM_THREAD_T));

    if(pThread)
    {

        memset(pThread, 0, sizeof(AM_THREAD_T));
        pThread->thread_fn = thread_fn;
        pThread->thread_arg = arg;
    
        s = pthread_attr_init(&pThread->attr);

        if(0 == s)
        {
            printf("creating thread...");
        
            s = pthread_create(&pThread->thread_id, &pThread->attr,
                   thread_fn, arg);

        }
        else
        {
            printf("pthread_create error =%d\n", s);
        
        }

    }
    return pThread;
}


void * am_net_alloc_client(void)
{
	struct sockaddr_in *pClient = NULL;

	pClient = (struct sockaddr_in *)AM_MALLOC(sizeof(struct sockaddr_in));

	return pClient;

}


