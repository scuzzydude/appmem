#include "appmemcpp.h"

CAppMem::CAppMem()
{
	cap_count = 0;
	pBaseCaps = NULL;
	bReady = false;
}

CAppMem::CAppMem(char *am_name)
{
	cap_count = 0;
	pBaseCaps = NULL;
	bReady = false;

	initBase(am_name, AM_TYPE_BASE_APPMEM);
}

const char * CAppMem::driverName(void)
{
	return amEntry.am_name;

}


AM_RETURN CAppMem::configCaps(AM_MEM_CAP_T *pCap)
{
	return AM_RET_GOOD;
}


void CAppMem::initBase(char *am_name, UINT32 amType)
{

	if(AM_RET_GOOD == am_get_entry_point(am_name, &amEntry))
	{
	
		cap_count = amEntry.get_cap_count(&amEntry);
		if(cap_count)
		{
			AM_MEM_CAP_T *pAllCaps;

			pAllCaps = new AM_MEM_CAP_T[cap_count];

			if(pAllCaps)
			{

				if(AM_RET_GOOD == amEntry.get_capabilities(&amEntry, pAllCaps, cap_count))
				{
					if(AM_TYPE_BASE_APPMEM == amType)
					{
						pBaseCaps = pAllCaps;
						bReady = true;
					}
					else 
					{

						for(UINT32 i = 0; i < cap_count; i++)
						{
							if(pAllCaps[i].amType == amType)
							{
								pBaseCaps = new AM_MEM_CAP_T;
								memcpy(pBaseCaps, (void *)&pAllCaps[i], sizeof(AM_MEM_CAP_T));
								cap_count = 1;
								break;
							}
						
						
						}
						
						delete pAllCaps;
						pAllCaps = NULL;

						if(NULL != pBaseCaps)
						{

							pCalls = (AM_FUNC_CALLS_T *)AM_MALLOC( sizeof(am_fn) * pBaseCaps->functionCount);
							if(pCalls)
							{
								amFunc.fn = pCalls;
								AM_MEM_CAP_T configCap;
								memcpy(&configCap, pBaseCaps, sizeof(AM_MEM_CAP_T));
								
								if(AM_RET_GOOD == configCaps(&configCap))
								{
									if(AM_RET_GOOD == amEntry.create_function(&amEntry, &configCap, &amFunc))
									{
										bReady = true;
									}
								}
							
							}
						}
					}	
				}
			}
		}
	}
}



AM_RETURN CAppMemFlat::configCaps(AM_MEM_CAP_T *pCap)
{

	/* TODO: Validate Address sizes are power of two */
	
	pCap->typeSpecific[TS_FLAT_ADDRESS_BYTE_SIZE] = address_size;
	pCap->maxSize = this->mem_size;

	return AM_RET_GOOD;

}
	
	
CAppMemFlat::CAppMemFlat(char *am_name, UINT32 memSize, UINT8 addressSize)
{
	address_size = addressSize;
	mem_size = memSize;

	initBase(am_name, AM_TYPE_FLAT_MEM);

}

AM_RETURN CAppMemFlat::write32(UINT32 offset, UINT32 val)
{
	if(sizeof(UINT32) == address_size)
	{
		return pCalls->write_al(amFunc.handle, &offset, &val); 
	}
	else
	{
		/* TODO: unoptimized call by caller, but still should work using multi-byte write*/
		AM_ASSERT(0);
		return AM_RET_PARAM_ERR;
	}
}

AM_RETURN CAppMemFlat::read32(UINT32 offset, UINT32 *pVal)
{
	if(sizeof(UINT32) == address_size)
	{
		return pCalls->read_al(amFunc.handle, &offset, pVal);	
	}
	else
	{
		/* TODO: unoptimized call by caller, but still should work using multi-byte write*/
		AM_ASSERT(0);
		return AM_RET_PARAM_ERR;
	}
}
