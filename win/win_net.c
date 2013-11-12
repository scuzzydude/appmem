// Client program example
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "appmem_net.h"

// TCP socket type
//#define DEFAULT_PROTO SOCK_STREAM
#define DEFAULT_PROTO SOCK_DGRAM

typedef struct _am_win_socket
{
	WSADATA w;								
	SOCKET sd;								
	struct sockaddr_in server;				/* Information about the server */
	struct sockaddr_in client;				/* Information about the client */
	int server_length;						/* Length of server struct */
	struct hostent *hp;						/* Information about the server */
	unsigned short port_number;				/* The port number to use */

} AM_WIN_SOCKET_T;

AM_RETURN am_net_establish_socket(AMLIB_ENTRY_T *pEntry, UINT32 ipaddr, UINT16 port)
{
	char host_name[256];					
	AM_WIN_SOCKET_T       *pSocket;

	pSocket = (AM_WIN_SOCKET_T *)malloc(sizeof(AM_WIN_SOCKET_T));

	if(NULL == pSocket)
	{
		return AM_RET_ALLOC_ERR;
	}
	else
	{
		memset(pSocket, 0, sizeof(AM_WIN_SOCKET_T));
	}
	
	
	if(0 == port)
	{
		pSocket->port_number = 4950;				

	}
	/*
	else
	{
		pSocket->port_number = port;				
	}
	*/

	if (WSAStartup(0x0101, &pSocket->w) != 0)
	{
		return AM_RET_SOCKET_ERR;
	}

	pSocket->sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (pSocket->sd == INVALID_SOCKET)
	{
		WSACleanup();
		return AM_RET_SOCKET_ERR;
	}


	pSocket->server.sin_family = AF_INET;
	pSocket->server.sin_port = htons(pSocket->port_number);
	/* Server Address */
	pSocket->server.sin_addr.S_un.S_addr = htonl(ipaddr);

	/* Client config */
	pSocket->client.sin_family = AF_INET;
	
	if(0 == port)
	{
		pSocket->client.sin_port = htons(1117);
	}
	else
	{
		pSocket->client.sin_port = htons(port);
	}
	
	printf("Client Port = %d\n", pSocket->client.sin_port);

	gethostname(host_name, sizeof(host_name));
	pSocket->hp = gethostbyname(host_name);

	if (NULL == pSocket->hp)
	{
			fprintf(stderr, "Could not get host name.\n");
			closesocket(pSocket->sd);
			WSACleanup();
			exit(0);
	}

	pSocket->client.sin_addr.S_un.S_un_b.s_b1 = pSocket->hp->h_addr_list[0][0];
	pSocket->client.sin_addr.S_un.S_un_b.s_b2 = pSocket->hp->h_addr_list[0][1];
	pSocket->client.sin_addr.S_un.S_un_b.s_b3 = pSocket->hp->h_addr_list[0][2];
	pSocket->client.sin_addr.S_un.S_un_b.s_b4 = pSocket->hp->h_addr_list[0][3];
	
	/* Bind */
	if (bind(pSocket->sd, (struct sockaddr *)&pSocket->client, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "Cannot bind address to socket.\n");
		closesocket(pSocket->sd);
		WSACleanup();
		return AM_RET_SOCKET_ERR;
	
	}


	/* Tranmsit data to get time */
	pSocket->server_length = sizeof(struct sockaddr_in);
	pEntry->pTransport = pSocket;
	return 0;
}

AM_RETURN am_net_recv_msg(void *pTransport, void *pMsg, UINT32 len, UINT32 *rcv_bytes)
{
	AM_WIN_SOCKET_T       *pSocket = (AM_WIN_SOCKET_T *)pTransport;
	int error;
	AM_ASSERT(pSocket);


	error = recvfrom(pSocket->sd, pMsg, len, 0, (struct sockaddr *)&pSocket->server, &pSocket->server_length);
	
	if(error < 0)
	{
		printf("Error receiving data =%d.\n", error);
		return AM_RET_IO_ERR;
	}

	return AM_RET_GOOD;


}



AM_RETURN am_int_send_msg(void *pTransport, void *pMsg, UINT32 len)
{
	AM_WIN_SOCKET_T       *pSocket = (AM_WIN_SOCKET_T *)pTransport;

	AM_ASSERT(pSocket);

	if (sendto(pSocket->sd, pMsg, len, 0, (struct sockaddr *)&pSocket->server, pSocket->server_length) == -1)
	{
		printf("Error transmitting data =%d.%s\n" );
		
		
		return AM_RET_IO_ERR;
	}

	return AM_RET_GOOD;
}

AM_RETURN am_net_destroy_socket(AMLIB_ENTRY_T *pEntry)
{
	/* TODO: close socket */
	/* free pSocket */

	return AM_RET_GOOD;

}


/***************************************************************************************/
/* Thread Specific stuff - move to another file later                                 **/
/***************************************************************************************/


typedef struct _am_thread
{

    HANDLE threadHandle;
    DWORD dwThreadId;
    am_fn_thread     thread_fn;
    void             *thread_arg;
    

} AM_THREAD_T;


DWORD WINAPI amThreadProc(  _In_  LPVOID lpParameter)
{
    AM_THREAD_T *pThread = lpParameter;
	AM_ASSERT(pThread);
	
	pThread->thread_fn(pThread->thread_arg);

#if 0
	int count = 0;
	while(1)
	{
		printf("amThreadProc count=%d\n", count);
		count++;
		Sleep(1000);
	}
#endif
	return 0;
}

void * am_thread_create(am_fn_thread thread_fn, void *arg)
{

    AM_THREAD_T *pThread = NULL;
  
    printf("am_thread_create()\n");


    pThread = (AM_THREAD_T *)AM_MALLOC(sizeof(AM_THREAD_T));

    if(pThread)
    {

        memset(pThread, 0, sizeof(AM_THREAD_T));

        pThread->thread_fn = thread_fn;
        pThread->thread_arg = arg;

        pThread->threadHandle = CreateThread(NULL, //Choose default security
                                    0, //Default stack size
                                    (LPTHREAD_START_ROUTINE)&amThreadProc,//Routine to execute
                                    (LPVOID) pThread, //Thread parameter
                                    0, //Immediately run the thread
                                    &pThread->dwThreadId //Thread Id
                                    );
        
		if(NULL == pThread->threadHandle)
		{
			printf("create thread error\n");
			AM_FREE(pThread);
			pThread = NULL;
		}

	
	
	}
    return (void *)pThread;
}


