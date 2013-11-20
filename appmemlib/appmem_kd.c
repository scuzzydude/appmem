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



#include "appmemlib.h"
#include "appmem_virtd.h"
#include "appmemd_ioctl.h"
#include "appmem_kd.h"

AM_RETURN am_kd_entry_close(AMLIB_ENTRY_T *pEntry)
{
	return AM_RET_GOOD;
}


#ifdef _WIN32
UINT32 am_kd_get_capabilities_count(AMLIB_ENTRY_T *pEntry){ return 0; }
AM_RETURN am_kd_get_capabilities(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count) { return 0; }
AM_RETURN am_kd_create_function(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc) { return 0; }


#else

int am_kd_close_driver(int fd)
{
	return close(fd);
}

int am_kd_open_driver(AMLIB_ENTRY_T *pEntry)
{
	int fd = open(pEntry->am_name, 0);
	
	if(fd < 0)
	{
		printf("DEBUG: FD=%d Error opening %s\n", fd, pEntry->am_name);
	}
	
	return fd;

}

AM_RETURN am_kd_get_capabilities(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pAmCaps, UINT32 count)
{
	int c;
	int fd;
	APPMEM_CMD_COMMON_T cmd;
	UINT32 len = count * sizeof(AM_MEM_CAP_T);
	fd = open(pEntry->am_name, 0);
	
	if(fd > -1)
	{
		cmd.op = AM_OP_CODE_GET_CAPS;
		cmd.len = len;
		cmd.data = (UINT64)pAmCaps;

		c = ioctl(fd, APPMEMD_OP_COMMON, &cmd);	

		
		if(c < 0)
		{
			printf("DEBUG: C=%d Error IOCTL %s\n", c, pEntry->am_name);
			return AM_RET_IO_ERR;
		}
		
		am_kd_close_driver(fd);
	
	}
	else
	{
		return AM_RET_OPEN_ERR;
	}


	return AM_RET_GOOD;

}


UINT32 am_kd_get_capabilities_count(AMLIB_ENTRY_T *pEntry)
{
	int c;
	int fd;
	UINT32 count = 0;
	APPMEM_CMD_COMMON_T cmd;

	fd = open(pEntry->am_name, 0);
	
	if(fd > -1)
	{
		cmd.op = AM_OP_CODE_GETC_CAP_COUNT;
		cmd.len = sizeof(UINT32);
		cmd.data = (UINT64)&count;
		
		c = ioctl(fd, APPMEMD_OP_COMMON, &cmd);	

		if(c < 0)
		{
			printf("DEBUG: C=%d Error IOCTL %s\n", c, pEntry->am_name);
		}
		else
		{
			printf("CAP_COUNT = %d\n", count);
		}

		am_kd_close_driver(fd);

	}
	
	return count;

}

AM_RETURN am_kd_write32_align(AM_HANDLE handle, void * p1, void *p2)
{
	APPMEM_CMD_ALIGNED_T cmd;
	int c;
	cmd.op = AM_OP_CODE_WRITE_ALIGN;
	cmd.offset = *(UINT32 *)p1;
	cmd.data = *(UINT32 *)p2;

	c = ioctl(handle, APPMEMD_OP_COMMON, &cmd);	
	
	if(c < 0)
	{
		return AM_RET_IO_ERR;
	}
	return AM_RET_GOOD;	

}

AM_RETURN am_kd_read32_align(AM_HANDLE handle, void * p1, void *p2)
{
	APPMEM_CMD_ALIGNED_T cmd;
	int c;
	cmd.op = AM_OP_CODE_READ_ALIGN;
	cmd.offset = *(UINT32 *)p1;
	cmd.data = (UINT64)p2;

	c = ioctl(handle, APPMEMD_OP_COMMON, &cmd);	
	
	if(c < 0)
	{
		return AM_RET_IO_ERR;
	}
	return AM_RET_GOOD;	

}

AM_RETURN am_kd_fixed_write_packet(AM_HANDLE handle, void * p1, void *p2)
{
    AM_MEM_FUNCTION_T *pFunc = handle;
	APPMEM_CMD_FIXED_PACKET_T cmd;
	int c = -1;
	cmd.op = AM_OP_CODE_WRITE_FIX_PACKET;

    //printf("write_packet - %p idx_size=%d\n", p1, pFunc->crResp.idx_size);
    //printf("write_packet - %p data_size=%d\n", p2, pFunc->crResp.data_size);
    

    memcpy(&cmd.data[0], p1, pFunc->crResp.idx_size);
    memcpy(&cmd.data[pFunc->crResp.pack_DataOffset], p2, pFunc->crResp.data_size);

    //printf("write_packet = QWORD size =%d, IOCTL=%08x\n", pFunc->crResp.wr_pack_qword_size, APPMEMD_OP_PACKET(pFunc->crResp.wr_pack_qword_size));
    
    
	c = ioctl(pFunc->handle, APPMEMD_OP_PACKET(pFunc->crResp.wr_pack_qword_size), &cmd);	
	
	if(c < 0)
	{
	    printf("IOCTL error - fixed_write_packet\n");
		return AM_RET_IO_ERR;
	}
	return AM_RET_GOOD;	


}


AM_RETURN am_kd_fixed_read_packet(AM_HANDLE handle, void * p1, void *p2)
{
    AM_MEM_FUNCTION_T *pFunc = handle;
	APPMEM_CMD_FIXED_PACKET_T cmd;
	int c = -1;
	cmd.op = AM_OP_CODE_READ_FIX_PACKET;

//    printf("read_packet - %p idx_size=%d\n", p1, pFunc->crResp.idx_size);
//    printf("read_packet - %p data_size=%d\n", p2, pFunc->crResp.data_size);
    

    memcpy(&cmd.data[0], p1, pFunc->crResp.idx_size);

    *(UINT64 *)(&cmd.data[pFunc->crResp.pack_DataOffset]) = (UINT64)p2;

//    printf("read_packet = QWORD size =%d, IOCTL=%08x\n", pFunc->crResp.rd_pack_qword_size, APPMEMD_OP_PACKET(pFunc->crResp.wr_pack_qword_size));
    
    
	c = ioctl(pFunc->handle, APPMEMD_OP_PACKET(pFunc->crResp.rd_pack_qword_size), &cmd);	
	
	if(c < 0)
	{
	    printf("IOCTL error - fixed_write_packet\n");
		return AM_RET_IO_ERR;
	}
	return AM_RET_GOOD;	


}



AM_RETURN am_kd_create_function(AMLIB_ENTRY_T *pEntry, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc) 
{ 
	AM_FUNC_CALLS_T *fn_array =  NULL;
	int c;
	int fd;
	APPMEM_CMD_BIDIR_T cmd;

	fd = open(pEntry->am_name, 0);
	
	if(fd > -1)
	{
		cmd.op = AM_OP_CODE_CREATE_FUNC;
		cmd.len_in = sizeof(AM_MEM_CAP_T);
		cmd.data_in = (UINT64)pCap;
		cmd.len_out = sizeof(APPMEM_RESP_CR_FUNC_T);
		cmd.data_out = (UINT64)&pFunc->crResp;

		c = ioctl(fd, APPMEMD_OP_BIDIR, &cmd);	

		if(c < 0)
		{
			printf("DEBUG: C=%d Error IOCTL %s\n", c, pEntry->am_name);
			return AM_RET_IO_ERR;
		}
		else
		{
			printf("CREATE FUNCTION GOOD = %d\n", 0);
			printf("CR RESP name=%s devt=%d\n", pFunc->crResp.am_name, pFunc->crResp.devt);
		
			pFunc->handle = -1; //closed
		
			fn_array = pFunc->fn;

			if(fn_array)
			{
				fn_array->open = am_kd_open;
				fn_array->close = am_kd_close;

                
                if(AM_OP_CODE_WRITE_ALIGN == pFunc->crResp.acOps[ACOP_WRITE])
                {
                    /* TODO - depending on address size might be different pointers */
    				fn_array->write_al = am_kd_write32_align;
                }
                else if(AM_OP_CODE_WRITE_FIX_PACKET == pFunc->crResp.acOps[ACOP_WRITE])
                {
                    printf("write_al = am_kd_fixed_write_packet\n");
                    fn_array->write_al = am_kd_fixed_write_packet;

                }
                

                if(AM_OP_CODE_READ_ALIGN == pFunc->crResp.acOps[ACOP_READ])
                {
                    /* TODO - depending on address size might be different pointers */
			        fn_array->read_al = am_kd_read32_align;
                }
                else if(AM_OP_CODE_READ_FIX_PACKET == pFunc->crResp.acOps[ACOP_READ])
                {
                    fn_array->read_al = am_kd_fixed_read_packet;
                }   
                    

			}
		
		}

		am_kd_close_driver(fd);

	}



	return AM_RET_GOOD;

}

AM_RETURN am_kd_open(void *p1)
{
	AM_MEM_FUNCTION_T *pFunc = p1;
	//full path
	char full_path[64];
	sprintf(full_path, "/dev/%s", pFunc->crResp.am_name);

	printf("opening %s\n", full_path);

	int fd = open(full_path, 0);
	
	if(fd > -1)
	{
		printf("Opened %s Handle = %d\n", full_path, fd);
		pFunc->handle = fd;
		return AM_RET_GOOD;
	}

	return AM_RET_INVALID_HDL;

}
AM_RETURN am_kd_close(void *p1)
{
//	AM_MEM_FUNCTION_T *pFunc = p1;

	return AM_RET_GOOD;
}




#endif
