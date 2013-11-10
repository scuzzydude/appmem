#include "appmemlib.h"
#include "appmemd_ioctl.h"
#include "am_k.h"
#include "am_k_sock.h"


#if 0
  3 
  4 #define EPERM            1      /* Operation not permitted */
  5 #define ENOENT           2      /* No such file or directory */
  6 #define ESRCH            3      /* No such process */
  7 #define EINTR            4      /* Interrupted system call */
  8 #define EIO              5      /* I/O error */
  9 #define ENXIO            6      /* No such device or address */
 10 #define E2BIG            7      /* Argument list too long */
 11 #define ENOEXEC          8      /* Exec format error */
 12 #define EBADF            9      /* Bad file number */
 13 #define ECHILD          10      /* No child processes */
 14 #define EAGAIN          11      /* Try again */
 15 #define ENOMEM          12      /* Out of memory */
 16 #define EACCES          13      /* Permission denied */
 17 #define EFAULT          14      /* Bad address */
 18 #define ENOTBLK         15      /* Block device required */
 19 #define EBUSY           16      /* Device or resource busy */
 20 #define EEXIST          17      /* File exists */
 21 #define EXDEV           18      /* Cross-device link */
 22 #define ENODEV          19      /* No such device */
 23 #define ENOTDIR         20      /* Not a directory */
 24 #define EISDIR          21      /* Is a directory */
 25 #define EINVAL          22      /* Invalid argument */
 26 #define ENFILE          23      /* File table overflow */
 27 #define EMFILE          24      /* Too many open files */
 28 #define ENOTTY          25      /* Not a typewriter */
 29 #define ETXTBSY         26      /* Text file busy */
 30 #define EFBIG           27      /* File too large */
 31 #define ENOSPC          28      /* No space left on device */
 32 #define ESPIPE          29      /* Illegal seek */
 33 #define EROFS           30      /* Read-only file system */
 34 #define EMLINK          31      /* Too many links */
 35 #define EPIPE           32      /* Broken pipe */
 36 #define EDOM            33      /* Math argument out of domain of func */
 37 #define ERANGE          34      /* Math result not representable */
 38 
 #endif
 


#include <linux/net.h>
#include <linux/in.h>
#include <linux/netpoll.h>
#define MESSAGE_SIZE 1024
//#define INADDR_SEND ((unsigned long int)0x0a00020f) //10.0.2.15
static bool sock_init;
static struct socket *sock;
static struct sockaddr_in sin;
static struct msghdr msg;
static struct iovec iov;
mm_segment_t old_fs;
char message[MESSAGE_SIZE]; 

void am_k_sock_init(unsigned long ipaddr)
{
    int error;
    int len;
    printk("appmemd: am_k_sock_init 0x%016lx = IP %d.%d.%d.%d \n", ipaddr, 
    (UINT8)((ipaddr >> 24) & 0xFF),
    (UINT8)((ipaddr >> 16) & 0xFF),
    (UINT8)((ipaddr >> 8) & 0xFF),
    (UINT8)((ipaddr >> 0) & 0xFF));

    error = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);

    
    if(error < 0)
    {
        printk(KERN_DEBUG "Can't create socket. Error %d\n",error);
        return;
    }
    else
    {
        printk("socket_create GOOD %d\n", error);
    }

    memset(&sin, 0, sizeof(struct sockaddr_in));

    /* Connecting the socket */
    sin.sin_family = AF_INET;
    sin.sin_port = htons(4950);
    sin.sin_addr.s_addr = htonl(ipaddr);

    error = sock->ops->connect(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr), 0);

    if(error < 0)
    {
        printk(KERN_DEBUG "Can't connect socket. Error %d\n",error);
        return;
    }
    else
    {
        printk("sock connect() GOOD %d\n", error);        
        printk("sin_port = %d\n", sin.sin_port);
        printk("sin.sin_addr.s_addr = %08x\n", sin.sin_addr.s_addr);

    }
    /* Preparing message header */
    msg.msg_flags = 0;
    msg.msg_name = &sin;
    msg.msg_namelen  = sizeof(struct sockaddr_in);
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_iov = &iov;
    msg.msg_control = NULL;
    sock_init = true;


    sprintf(message, "Brandon test message to IP %016lx Port %08x\n", ipaddr, sin.sin_port);
    

    iov.iov_base = message;
    len = strlen(message);
    iov.iov_len = len;
    msg.msg_iovlen = len;

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    error = sock_sendmsg(sock,&msg,len);
    set_fs(old_fs);

    if(error < 0)
    {
        printk("Error sock_sendmsg %d\n", error);
    }
    else
    {
        printk("Send Message good %d\n", error);
    }



}

