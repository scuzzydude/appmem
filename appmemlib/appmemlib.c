#include "appmemlib.h"
#include "appmem_virtd.h"

char * AmCapTypeStr[2] = {
	"Flat Memory",
	"Static Array"
};



UINT32 am_get_capabilities_count(char *am_name)
{
	UINT32 count = 0;

	if(NULL == am_name)
	{
		count = am_virtd_get_capabilites_count();
	}
	else
	{
		/* TODO: Open driver and query */	
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
	