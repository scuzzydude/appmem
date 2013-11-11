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
#define DEFAULT_PROTO SOCK_STREAM
//#define DEFAULT_PROTO SOCK_DGRAM

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
	
	pSocket->port_number = DEFAULT_AM_NET_PORT;				

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
	pSocket->client.sin_port = htons(0);
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
		printf("Error transmitting data.\n");
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
