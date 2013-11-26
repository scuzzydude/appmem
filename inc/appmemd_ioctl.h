/*****************************************************************************************

Copyright (c) 2013, Brandon Awbrey
All rights reserved.

https://github.com/scuzzydude/appmem



Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************************/
#ifndef _APPMEMD_IOCTL_H
#define _APPMEMD_IOCTL_H
#ifdef _APPMEMD
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */






#endif

struct am_mem_function_t;

#define AM_FUNC struct am_mem_function_t

#ifdef _APPMEMD
struct _appmem_kdevice;

#define GET32_FROM_USER(_fd, _locaddr, _uaddr) do { \
                                               if(((AM_FUNC_DATA_U *)_fd)->common.flags & AM_FUNC_DATA_FLAG_MMAPPED) \
                                               { \
                                                    *(UINT32 *) _locaddr = *(UINT32 *)_uaddr; \
                                               } \
                                               else \
                                               { \
                                                    __get_user(*(UINT32 *)_locaddr, (UINT32 *)_uaddr); \
                                               }} while(0)

#define PUT32_TO_USER(_fd, _locaddr, _uaddr) do { \
                                               if(((AM_FUNC_DATA_U *)_fd)->common.flags & AM_FUNC_DATA_FLAG_MMAPPED) \
                                               { \
                                                    *(UINT32 *)_uaddr = *(UINT32 *)_locaddr; \
                                               } \
                                               else \
                                               { \
                                                    __put_user(*(UINT32 *)_locaddr, (UINT32 *)_uaddr); \
                                               }} while(0)


#define COPY_FROM_USER(_fd, _locaddr, _uaddr, _len) do { \
                                               if(((AM_FUNC_DATA_U *)_fd)->common.flags & AM_FUNC_DATA_FLAG_MMAPPED) \
                                               { \
                                                    memcpy(_locaddr, _uaddr, _len);\
                                               } \
                                               else \
                                               { \
                                                    copy_from_user(_locaddr, _uaddr, _len);\
                                               }} while(0)

#define COPY_TO_USER(_fd, _uaddr, _locaddr, _len) do { \
                                               if(((AM_FUNC_DATA_U *)_fd)->common.flags & AM_FUNC_DATA_FLAG_MMAPPED) \
                                               { \
                                                    memcpy(_uaddr, _locaddr, _len);\
                                               } \
                                               else \
                                               { \
                                                    if(copy_from_user(_locaddr, _uaddr, _len)){};\
                                               }} while(0)
#else
#define GET32_FROM_USER(_fd, _locaddr, _uaddr) 	*(UINT32 *) _locaddr = *(UINT32 *)_uaddr
#define PUT32_TO_USER(_fd, _locaddr, _uaddr) *(UINT32 *)_uaddr = *(UINT32 *)_locaddr
#define COPY_FROM_USER(_fd, _locaddr, _uaddr, _len) memcpy(_locaddr, _uaddr, _len)
#define COPY_TO_USER(_fd, _uaddr, _locaddr, _len) memcpy(_uaddr, _locaddr, _len)

#endif

#define AM_K_MAP_PI_START 0xAA3EBB11
#define AM_K_MAP_CI_START 0x74000001


#define AM_OP_CODE_MASK             0x000000FF


#define AM_OP_MAX_OPS               64


#define AM_FUNC_PACK_TYPE_FLAG_RESP    0x80
#define AM_FUNC_PACK_TYPE_FLAG_ERR     0x40


#define AM_NET_GET_PACKTYPE(_x) ((_x >> 16) & 0x3F)
#define AM_SET_PACKTYPE(_x, _op) ( (    (_x & 0x3F) << 16 ) | (_op & AM_OP_CODE_MASK)     )

#define AM_PACK_TYPE_OPCODE_ONLY       0x01
#define AM_PACK_TYPE_FIVO              0x02
#define AM_PACK_ALIGN                  0x03
#define AM_PACK_ACTION                 0x04
#define AM_KPACK                       0x05



#define AM_OP_FLAG_OP_ONLY          0x00000000
#define AM_OP_FLAG_COMMON           0x01000000    /* Commands That just have single data buffer and length */
#define AM_OP_FLAG_BIDIR            0x03000000    /* Data Buffer in and buffer out */
#define AM_OP_FLAG_PACKET           0x04000000
#define AM_OP_ALIGNED               0x80000000


#define IS_OP_ALIGNED(_x) (AM_PACK_ALIGN == AM_NET_GET_PACKTYPE(_x))
#define IS_OP_PACKET(_x)  (AM_KPACK == AM_NET_GET_PACKTYPE(_x)) 

#define AM_OP_IDENTIFY                        0x1
#define AM_OP_GET_CAP_COUNT                   0x2
#define AM_OP_GET_CAP                         0x3
#define AM_OP_CREATE_FUNC                     0x4
#define AM_OP_RELEASE_FUNC                    0x5
#define AM_OP_OPEN_FUNC                       0x6
#define AM_OP_CLOSE_FUNC                      0x7
#define AM_OP_SORT                            0x8


#define AM_OP_WRITE_ALIGN                    0x10
#define AM_OP_READ_ALIGN                     0x11

#define AM_OP_CODE_GETC_CAP_COUNT   AM_SET_PACKTYPE(AM_PACK_TYPE_OPCODE_ONLY, AM_OP_GET_CAP_COUNT)
#define AM_OP_CODE_GET_CAPS         AM_SET_PACKTYPE(AM_PACK_TYPE_FIVO, AM_OP_GET_CAP)
#define AM_OP_CODE_CREATE_FUNC      AM_SET_PACKTYPE(AM_PACK_TYPE_FIVO, AM_OP_CREATE_FUNC)
#define AM_OP_CODE_RELEASE_FUNC     AM_SET_PACKTYPE(AM_PACK_TYPE_OPCODE_ONLY, AM_OP_RELEASE_FUNC)

#define AM_OP_CODE_WRITE_ALIGN      AM_SET_PACKTYPE(AM_PACK_ALIGN, AM_OP_WRITE_ALIGN) 
#define AM_OP_CODE_READ_ALIGN       AM_SET_PACKTYPE(AM_PACK_ALIGN, AM_OP_READ_ALIGN)

#define AM_OP_CODE_WRITE_FIX_PACKET AM_SET_PACKTYPE(AM_KPACK, AM_OP_WRITE_ALIGN)
#define AM_OP_CODE_READ_FIX_PACKET  AM_SET_PACKTYPE(AM_KPACK, AM_OP_READ_ALIGN) 


#define AM_OPCODE(x) (AM_OP_CODE_MASK & x)



typedef struct _appmem_cmd_common_t
{
	UINT32 op;
	UINT32 len;
	UINT64 data;

} APPMEM_CMD_COMMON_T;


typedef struct _appmem_cmd_aligned
{
	UINT32 op;
	UINT32 offset;
	UINT64 data;

} APPMEM_CMD_ALIGNED_T;


typedef struct _appmem_cmd_bidir_t 
{
	UINT32 op;
	UINT32 len_in;
	UINT64 data_in;
	UINT32 len_out;
	UINT32 flags;
	UINT64 data_out;
}	APPMEM_CMD_BIDIR_T;

#define APPMEM_PACKET_DATA_DWORDS ( ( 256 - sizeof(UINT32)) / sizeof(UINT32)) 

typedef struct _appmem_cmd_fixed_packet_t 
{
	UINT32 op;
	UINT32 data[APPMEM_PACKET_DATA_DWORDS];
} APPMEM_CMD_FIXED_PACKET_T;


typedef union _appmem_cmd_u
{
	APPMEM_CMD_COMMON_T        common;
	APPMEM_CMD_BIDIR_T         bidir;
	APPMEM_CMD_ALIGNED_T       aligned;
	APPMEM_CMD_FIXED_PACKET_T  packet;

} APPMEM_CMD_U;


typedef struct _appmem_kam_cmd
{
    APPMEM_CMD_U cmd;

} APPMEM_KAM_CMD_T;








typedef AM_RETURN (*am_cmd_fn)(AM_FUNC *pFunc, void *p1);
typedef AM_RETURN (*am_fn)(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2);
typedef AM_RETURN (*am_fn_action) (AM_FUNC *pFunc, void * p1, UINT64 l1);
typedef AM_RETURN (*am_fn_align)(AM_FUNC *pFunc, void * p1, void *p2);
typedef AM_RETURN (*am_fn_raw)(void * p1);
typedef AM_RETURN (*am_fn_op_only)(AM_FUNC *pFunc, void *p1, UINT32 *ret_len);
typedef AM_RETURN (*am_fivo) (AM_FUNC *pFunc, void *p1, UINT64 l1, void *p2, UINT32 *ret_len);
typedef AM_RETURN (*am_iter)(AM_FUNC *pFunc, void * p1, UINT64 l1, void *p2, UINT64 l2, UINT32 *iter_handle);


typedef union _am_fn_u
{
	am_cmd_fn      config;
	am_fn          func;
	am_fn_align    align;
	am_fn_raw      raw;
	am_fn_action   action;
	am_fn_op_only  op_only;
	am_fivo        fivo;	
	am_iter        iter;

} AM_FN_U;



#ifdef _APPMEMD

struct _appmem_kdevice;
union am_func_data; 



typedef struct _appmedk_mmap
{
    void                 *pMapped;
    unsigned long         req_len;
    unsigned long         map_len;
    int                   isMapped;
    struct task_struct   *pMapThread;
    

} APPMEMK_MMAP;


typedef struct _appmem_kdevice
{
    struct am_mem_function_t *pFunc;
    APPMEMK_MMAP           map;
    int                    minor;
    UINT32                 amType;
    dev_t                  devt;
    struct cdev            cdev;

    AM_FN_U                *pfnOps;     
    struct _appmem_kdevice *next;
//    union am_func_data     *pVdF;  
    char                   am_name[64];
      
    UINT8  pack_DataOffset; /* Offset in Dwords (i.e. Pack data structure is array of DWORDS */
    UINT8  wr_pack_size; /* Bytes - We transfer packets in multiples of 64 bits - driver checks by bytes */
    UINT8  rd_pack_size; /* Bytes - We transfer packets in multiples of 64 bits - driver check by bytes*/

} APPMEM_KDEVICE;


#endif

typedef enum amAccessOps
{
    ACOP_WRITE,
    ACOP_READ

} AM_ACCESS_OPS;

#define AM_MAX_ACCESS_OPS (2) 

typedef struct _appmem_resp_cr_func_t
{
	char am_name[64];
    UINT32 devt;
	UINT32 flags;
	UINT64 globalID;
	UINT32 ops[AM_OP_MAX_OPS];
    UINT32 acOps[AM_MAX_ACCESS_OPS];
    UINT32 data_size;
    UINT32 idx_size;

    UINT8  pack_DataOffset; /* Offset in Dwords (i.e. Pack data structure is array of DWORDS */
    UINT8  wr_pack_qword_size; /* We transfer packets in multiples of 64 bits */
    UINT8  rd_pack_qword_size; /* We transfer packets in multiples of 64 bits */
    

} APPMEM_RESP_CR_FUNC_T;



#ifndef _WIN32

#include <linux/ioctl.h>

/* Use 'K' as magic number */
#define APPMEMD_IOC_MAGIC  'K'

#define APPMEMD_IOCRESET    _IO(APPMEMD_IOC_MAGIC, 0)

#define APPMEMD_OP_COMMON   _IOWR(APPMEMD_IOC_MAGIC, 1, 1024)
#define APPMEMD_OP_BIDIR    _IOWR(APPMEMD_IOC_MAGIC, 3, 1024)

#define APPMEMD_OP_PACKET(_qwords) _IOWR(APPMEMD_IOC_MAGIC, _qwords, 1024)

#define APPMEMD_IOC_MAXNR 32


#endif 
#endif




