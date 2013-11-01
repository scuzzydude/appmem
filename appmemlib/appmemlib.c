#include "appmemlib.h"
#include "appmem_virtd.h"
#include "appmemd_ioctl.h"
#include "appmem_kd.h"


char * AmCapTypeStr[4] = {
	"Appmem Base Capabilites",
	"Flat Memory",
	"Static Array",
	"Associative Array"
};

AM_RETURN am_get_entry_point(char *am_name, AMLIB_ENTRY_T *pEntry)
{
	if(pEntry)
	{
		if(am_name)
		{
			pEntry->am_name = am_name;
			pEntry->create_function = am_kd_create_function;
			pEntry->get_capabilities = am_kd_get_capabilities;
			pEntry->get_cap_count = am_kd_get_capabilities_count;
		}
		else
		{
			pEntry->am_name = "virtd";
			pEntry->create_function = am_virtd_create_function;
			pEntry->get_capabilities = am_virtd_get_capabilities;
			pEntry->get_cap_count = am_virtd_get_capabilites_count;
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

