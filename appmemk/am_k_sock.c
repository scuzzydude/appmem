#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_k.h"
#include "am_k_sock.h"


#include <linux/net.h>
#include <linux/in.h>
#include <linux/netpoll.h>
#define MESSAGE_SIZE 1024
#define INADDR_SEND ((unsigned long int)0x0a00020f) //10.0.2.15
static bool sock_init;
static struct socket *sock;
static struct sockaddr_in sin;
static struct msghdr msg;
static struct iovec iov;


void am_k_sock_init(UINT32 ipaddr)
{
    printk("appmemd: am_k_sock_init 0x%08x = IP %d.%d.%d.%d \n", ipaddr, 
    ((ipaddr >> 24) & 0xFF),
    ((ipaddr >> 16) & 0xFF),
    ((ipaddr >> 8) & 0xFF),
    ((ipaddr >> 0) & 0xFF));
    

}

