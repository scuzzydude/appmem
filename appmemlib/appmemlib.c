#include "appmemlib.h"
#include "appmem_virtd.h"
#include "appmemd_ioctl.h"

char * AmCapTypeStr[2] = {
	"Flat Memory",
	"Static Array"
};



UINT32 am_get_capabilities_count(char *am_name)
{
	int c;
	int fd;
	UINT32 count = 44;
	APPMEM_CMD_COMMON_T cmd;

	cmd.cmd = 1;
	cmd.len = sizeof(APPMEM_CMD_COMMON_T);
	cmd.data = (UINT64)&count;

	if(NULL == am_name)
	{
		count = am_virtd_get_capabilites_count();
	}
	else
	{
			printf("DEBUG: Open %s\n", am_name);

		/* TODO: Open driver and query */	
#ifndef _WIN32
		fd = open(am_name, 0);
		if(fd < 0)
		{
			printf("DEBUG: FD=%d Error opening %s\n", fd, am_name);
			return 0;
		}
		else
		{
			printf("DEBUG: FD=%d opening compilete %s\n", fd, am_name);
			c = ioctl(fd, APPMEMD_OP_COMMON, &cmd);	

			if(c < 0)
			{
				printf("DEBUG: C=%d Error IOCTL %s\n", c, am_name);
			}
			else
			{
				printf("COUNT = %d\n", count);
			}

		}
#endif



	}


	return count;
}

AM_RETURN am_get_capabilities(char *am_name, AM_MEM_CAP_T *pAmCaps, UINT32 count)
{
	AM_RETURN error = AM_RET_GOOD;

	if((0 != count)  && (NULL != pAmCaps))
	{
		if(NULL == am_name)
		{
			error = am_virtd_get_capabilities(am_name, pAmCaps, count);

		}
		else
		{

		}
	
	}
	else
	{
		error = AM_RET_PARAM_ERR; 
	
	}


	return error;

}

UINT32 am_sprintf_capability(AM_MEM_CAP_T *pAmCap, char *buf, UINT32 buf_size)
{
	/* TODO : Make buff size safe */

	UINT32 bytes_written = 0;
	if(pAmCap)
	{
		if(buf_size && buf)
		{
			bytes_written += sprintf(buf, "MEMORY TYPE = 0x%08x : %s\n", pAmCap->amType, AmCapTypeStr[pAmCap->amType]); 
			bytes_written += sprintf(buf, "MAX_SIZE = 0x%016llx : %d MB\n", pAmCap->maxSize, pAmCap->maxSize / (1024 * 1024));

		}
		
	
	
	
	}

	return bytes_written;
}

AM_RETURN am_create_function(char *am_name, AM_MEM_CAP_T *pCap, AM_MEM_FUNCTION_T *pFunc)
{
	AM_RETURN error = 0;

	error = am_virtd_create_function(am_name, pCap, pFunc);
	
	return error;
}
	