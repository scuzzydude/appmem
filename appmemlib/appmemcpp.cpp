#include "appmemcpp.h"



/***************************************************************************/
/**  CAppMem (Base Class)                                                  */
/***************************************************************************/

CAppMem::CAppMem()
{
	cap_count = 0;
	pBaseCaps = NULL;
	bReady = false;
	pCalls = NULL;
}

CAppMem::~CAppMem()
{
	printf("Destructor : ~CAppMem()\n");

	if(pBaseCaps)
	{
		delete pBaseCaps;
	}
	if(pCalls)
	{
		delete pCalls;
	}

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

/***************************************************************************/
/**  CAppMemFlat                                                           */
/***************************************************************************/

	
CAppMemFlat::CAppMemFlat(char *am_name, UINT32 memSize, UINT8 addressSize)
{
	address_size = addressSize;
	mem_size = memSize;
	elements = 0;

	if(address_size)
	{
		elements = mem_size / address_size;
	}
	initBase(am_name, AM_TYPE_FLAT_MEM);

}

CAppMemFlat::~CAppMemFlat()
{
	printf("Destructor : ~CAppMemFlat()\n");
}
		

UINT32 CAppMemFlat::count(void)
{
	return elements;
}

AM_RETURN CAppMemFlat::configCaps(AM_MEM_CAP_T *pCap)
{

	/* TODO: Validate Address sizes are power of two */
	
	pCap->typeSpecific[TS_FLAT_ADDRESS_BYTE_SIZE] = this->address_size;
	pCap->maxSize = this->mem_size;

	return AM_RET_GOOD;

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

/*
A big challenging to directly use the [] = X mutator 
We'd have to provide caller scratch are to write the value to as well as cache 
The index/key.   Then on any read/write operation, we'd have to flush out the scratch to appmem */
/* This would be additional checks on the every read/write to see if scratch has valid data */
/* And would require locks on client */
/* TODO: Review later, for now [] operator are lookup only */
/*
UINT32& CAppMemFlat::operator[] (unsigned int idx) const
{

	return scratch_pad;
}
*/


UINT32& CAppMemFlat::operator[] (const unsigned int idx)
{
	if(idx < elements)
	{
		if(AM_RET_GOOD == read32(idx * address_size, &scratch_pad))
		{
			return scratch_pad;
		}
		else
		{
			/* TODO: Throw exception */
			AM_ASSERT(0);
			return scratch_pad;

		}
	}
	else
	{
		/* TODO: Throw exception */
		AM_ASSERT(0);
		return scratch_pad;
	}
	
}



/***************************************************************************/
/**  CAppMemStaticArray                                                    */
/***************************************************************************/

CAppMemStaticArray::CAppMemStaticArray(char *am_name, UINT32 elemCount, UINT32 dataSize, UINT32 indexSize)
{
	index_size = indexSize;
	data_size = dataSize;
	elements = elemCount;
	mem_size = dataSize * elemCount;

	
	initBase(am_name, AM_TYPE_ARRAY);


}

CAppMemStaticArray::~CAppMemStaticArray()
{

}

AM_RETURN CAppMemStaticArray::configCaps(AM_MEM_CAP_T *pCap)
{

	/* TODO : Power of two checking user passed in index size */
	if(pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE] & this->index_size)
	{
		pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE] = index_size;
		pCap->maxSize = elements;
		pCap->typeSpecific[TS_STAT_ARRAY_VAL_DATA_TYPES] = TS_STAT_DT_FIXED_WIDTH;
		pCap->typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE] = 4;
		return AM_RET_GOOD;

	}

	return AM_RET_PARAM_ERR;

}

UINT32 CAppMemStaticArray::count(void)
{
	return elements;
}

AM_RETURN CAppMemStaticArray::insert(UINT32 index, void *pVal)
{
	
	if(index < elements)
	{
		return pCalls->write_al(amFunc.handle, &index, pVal);
	}
	return AM_RET_KEY_OUT_OF_RANGE;

}
AM_RETURN CAppMemStaticArray::get(UINT32 index, void *pVal)
{
	if(index < elements)
	{
		return pCalls->read_al(amFunc.handle, &index, pVal);
	}
	return AM_RET_KEY_OUT_OF_RANGE;
}


/***************************************************************************/
/**  CAppMemAsscArray                                                      */
/***************************************************************************/
CAppMemAsscArray::CAppMemAsscArray(char *am_name, UINT32 keySize, UINT32 dataSize, bool bFKey, bool bFData, bool bODupe )
{
	key_size = keySize;
	data_size = dataSize;
	bFixedKey = bFKey;
	bFixedData = bFData;
	bErrorOnDupe = bODupe;

	initBase(am_name, AM_TYPE_ASSOC_ARRAY);


}
CAppMemAsscArray::~CAppMemAsscArray()
{

}

AM_RETURN CAppMemAsscArray::configCaps(AM_MEM_CAP_T *pCap)
{

	/* TODO : Power of two checking user passed in index size */
//	if(pCap->typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE] )
		if((pCap->typeSpecific[TS_ASSCA_KEY_MAX_SIZE] >= key_size) &&
			(pCap->typeSpecific[TS_ASSCA_DATA_MAX_SIZE] >=  data_size))

		{
			pCap->typeSpecific[TS_ASSCA_KEY_MAX_SIZE] = key_size;
			pCap->typeSpecific[TS_ASSCA_DATA_MAX_SIZE] = data_size;

			if(bFixedKey)
			{	
				pCap->typeSpecific[TS_ASSCA_KEY_TYPE] =  TS_ASSCA_KEY_FIXED_WIDTH;
			}
			
			if(bFixedData)
			{
				pCap->typeSpecific[TS_ASSCA_DATA_TYPE] =  TS_ASSCA_DATA_FIXED_WIDTH;
			}
		
		}
		return AM_RET_GOOD;

	return AM_RET_PARAM_ERR;

}

AM_RETURN CAppMemAsscArray::insert(void *pKey, void *pVal)
{
	if(bFixedKey && bFixedData)
	{
		return pCalls->write_al(amFunc.handle, pKey, pVal);
	}
	else
	{
		/* TODO -- handlers for non fixed */
		/* Actually, better ways to handle than to check config everytime */
		/* Inherit different insert()/get() based on constructor */
		
		return AM_RET_PARAM_ERR;
	}

}
AM_RETURN CAppMemAsscArray::get(void *pKey, void *pVal)
{
	return AM_RET_GOOD;
}
