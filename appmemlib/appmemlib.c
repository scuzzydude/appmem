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
#include <memory.h>
#include "appmemlib.h"
#include "appmem_virtd.h"
#include "appmemd_ioctl.h"
#include "appmem_kd.h"
#include "appmem_net.h"

char * AmCapTypeStr[5] = {
	"Appmem Base Capabilites",
	"Flat Memory",
	"Static Array",
	"Associative Array",
	"Linked List"
};

UINT32 am_get_ip_address(char *am_name)
{
	int  byte[4];
	union _ipaddr_t
	{
		UINT32 dw;
		UINT8 byte[4];
	} ipaddr_t;

	UINT32 len;
	UINT32 i;
	UINT32 dcnt = 0;


	/* TODO: IF it's IP Address -- there is probably a better way to do this */

	len = strlen(am_name);

	if(len > strlen("1.1.1.1"))
	{
		for(i = 0; i < len; i++)
		{
			if(am_name[i] == '.')
			{
				dcnt++;
			}
		}

		if(dcnt == 3)
		{
			sscanf(am_name, "%d.%d.%d.%d", 
				&byte[0],
				&byte[1],
				&byte[2],
				&byte[3]);
		}
		else
		{
			return 0;
		}
	}

	ipaddr_t.byte[3] = (UINT8) (0xFF & byte[0]);
	ipaddr_t.byte[2] = (UINT8) (0xFF & byte[1]);
	ipaddr_t.byte[1] = (UINT8) (0xFF & byte[2]);
	ipaddr_t.byte[0] = (UINT8) (0xFF & byte[3]);

	
	return ipaddr_t.dw;
}
AM_RETURN am_get_entry_point(char *am_name, AMLIB_ENTRY_T *pEntry)
{
	UINT32 ipaddr = 0;
	char *buf;

	if(pEntry)
	{

		if(am_name)
		{

			ipaddr = am_get_ip_address(am_name);
			printf("IP ADDR = %08x\n", ipaddr);

			if(0 != ipaddr)
			{
				buf = (char *)AM_MALLOC(64);

				sprintf(buf, "appmem%s", am_name);		
				pEntry->am_name = buf;
				
				if(AM_RET_GOOD != am_net_init_entry(pEntry, ipaddr))
				{
					printf("Error NET Init IP=%08x\n", ipaddr);
					pEntry->am_name = NULL;
					AM_FREE(buf);
					return AM_RET_IO_ERR;
				}

				pEntry->get_cap_count = am_net_get_capabilites_count;
				pEntry->get_capabilities = am_net_get_capabilities;
				pEntry->create_function = am_net_create_function; 
				pEntry->close = am_net_entry_close;
			
			}
			else
			{
				pEntry->am_name = am_name;
				pEntry->create_function = am_kd_create_function;
				pEntry->get_capabilities = am_kd_get_capabilities;
				pEntry->get_cap_count = am_kd_get_capabilities_count;
				pEntry->close = am_kd_entry_close;
				pEntry->get_cap_details = am_kd_get_cap_details;
				
			}
		}
		else
		{
			pEntry->am_name = "virtd";
			pEntry->create_function = am_virtd_create_function;
			pEntry->get_capabilities = am_virtd_get_capabilities;
			pEntry->get_cap_count = am_virtd_get_capabilites_count;
			pEntry->close = am_virtd_entry_close;
			pEntry->get_cap_details = am_virt_get_cap_details;
		}


	}
	else
	{
		return AM_RET_PARAM_ERR; 
	}

	return AM_RET_GOOD;

}



UINT32 am_sprintf_capability(AM_MEM_CAP_T *pAmCap, char *buf, UINT32 buf_size)
{
	UINT32 bytes_written = buf_size;
	int sb;
	int complete = 0;
	char *start_buf = buf;

	if(pAmCap)
	{
		if(buf_size && buf)
		{
			do
			{
				sb = AM_SNPRINTF(buf, buf_size, "MEMORY TYPE = 0x%08x : %s\n", pAmCap->amType, AmCapTypeStr[pAmCap->amType]);
				if(-1 == sb) break;
				buf_size -= sb;
				buf += sb;

				sb = AM_SNPRINTF(buf, buf_size, "MAX_SIZE    = 0x%016llx : %d MB\n", pAmCap->maxSize, pAmCap->maxSize / (1024 * 1024));
				if(-1 == sb) break;
				buf_size -= sb;
				buf += sb;

				sb = AM_SNPRINTF(buf, buf_size, "SUB_TYPES = = 0x%08x\n", pAmCap->subType);
				if(-1 == sb) break;
				buf_size -= sb;
				buf += sb;
			
				complete = 1;
					
			} while(0);

		}
	}

	if(complete)
	{
		bytes_written -= buf_size;

	}
	else
	{
		start_buf[bytes_written - 1] = '\0';
	}

	return bytes_written;
}


UINT32 am_sprintf_cap_details(AM_CAP_DETAILS *pCapDetails, char *buf, UINT32 buf_size)
{
	UINT32 bytes_written = buf_size;
	int sb;
	int complete = 0;
	char *start_buf = buf;
	UINT32 i;

	if(pCapDetails && buf_size && buf)
	{
		do
		{
			sb = AM_SNPRINTF(buf, buf_size, "amType  = 0x%08x : %s\n", pCapDetails->amType, pCapDetails->amTypeSz);
			if(-1 == sb) break;
			buf_size -= sb;
			buf += sb;

			sb = AM_SNPRINTF(buf, buf_size, "subType = 0x%08x : %s\n", pCapDetails->subType, pCapDetails->subTypeSz);
			if(-1 == sb) break;
			buf_size -= sb;
			buf += sb;

			sb = AM_SNPRINTF(buf, buf_size, "typeSpecificCount = 0x%08x \n", pCapDetails->typeSpecificCount);
			if(-1 == sb) break;
			buf_size -= sb;
			buf += sb;

			
			for(i = 0; i < pCapDetails->typeSpecificCount; i++)
			{
				sb = AM_SNPRINTF(buf, buf_size, "TS#%d = %s\n", i, pCapDetails->typeSpecificSz[i]);
				if(-1 == sb) break;
				buf_size -= sb;
				buf += sb;

			}

			if(i == pCapDetails->typeSpecificCount)
			{
				complete = 1;
			}


		} while(0);
	
	
		if(complete)
		{
			bytes_written -= buf_size;
		}
		else
		{
			start_buf[bytes_written - 1] = '\0';
		}
	
	}



	return bytes_written;
}


