#include <stdio.h>
#include <stdlib.h>
#include "appmemlib.h"
#include "am_test_os.h"
#include "am_assca.h"

void am_test_flat_mem(AM_MEM_CAP_T *pCap, AMLIB_ENTRY_T *pEntry)
{

	AM_MEM_CAP_T aCap;
	void *func_buf;
	AM_MEM_FUNCTION_T amFmf;
	UINT32 mem_size = 1024 * 1024;
	UINT32 i;
	UINT32 val;
	UINT32 *ptr32;
	UINT32 *local_buf;
	UINT32 handle;
	double elap1, elap2;
	INIT_OS_HR_TIMER(0);
	UINT32 random_ops = 10000000;
	UINT32 rval;
	UINT32 addr;
	

	memcpy(&aCap, pCap, sizeof(aCap));

	if(aCap.typeSpecific[TS_FLAT_ADDRESS_BYTE_SIZE] & 0x4)
	{
		aCap.typeSpecific[TS_FLAT_ADDRESS_BYTE_SIZE] = 0x4;
		aCap.maxSize = mem_size;
	}
	else
	{
		printf("NO 32bit Address Size\n");
		return;
	}

	func_buf = AM_MALLOC( sizeof(am_fn) * aCap.functionCount);
	amFmf.fn = func_buf;
	if(AM_RET_GOOD != pEntry->create_function(pEntry, &aCap, &amFmf))
	{
		printf("Create Function Error\n");
		return;
	}
		
	handle = amFmf.handle;

	if(AM_RET_GOOD != amFmf.fn->open(handle, NULL, 0, NULL, 0))
	{
		printf("Open Function Error\n");
		return;
	}


	local_buf = (UINT32 *)AM_MALLOC(mem_size);

	if(local_buf)
	{
		ptr32 = local_buf;
		
		OS_HR_TIMER_START();
		


		
		for( i = 0; i < mem_size ; i += 4)
		{
			*ptr32 = i;
			ptr32++;
		}
		OS_HR_TIMER_STOP();
		elap1 = OS_HR_TIMER_GET_ELAP();
		printf("WRITE %d bytes ELAP = %f\n", mem_size, elap1);

	
	}
	else
	{
		printf("LOcal buf error\n");
		return;
	}

	OS_HR_TIMER_START();

	for( i = 0; i < mem_size ; i += 4)
	{
		amFmf.fn->write_al(handle, &i, &i);
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("WRITE %d bytes ELAP = %f\n", mem_size, elap2);
	printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	

	val = 0;
	rval = 0;

	OS_HR_TIMER_START();
	for( i = 0; i < random_ops; i++)
	{
		ptr32 = (UINT32 *)((UINT32)local_buf + (((val + rval) % mem_size) & ~3)); 
		rval = rand();
		val = *ptr32;
	}
	OS_HR_TIMER_STOP();
	elap1 = OS_HR_TIMER_GET_ELAP();
	printf("READ %d ops ELAP = %f\n", random_ops, elap1);


	rval = 0;
	val = 0;
	OS_HR_TIMER_START();
	for( i = 0; i < random_ops; i++)
	{
		addr = (((val + rval) % mem_size) & ~3); 
		amFmf.fn->read_al(handle, &addr, &val);
		rval = rand();
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("READ %d ops  = %f\n", random_ops, elap2);
	printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	


	



	if(AM_RET_GOOD != amFmf.fn->close(handle, NULL, 0, NULL, 0))
	{
		printf("Close Function Error\n");
		return;
	}


}

void am_test_static_array(AM_MEM_CAP_T *pCap, AMLIB_ENTRY_T *pEntry)
{
	AM_MEM_CAP_T aCap;
	UINT32 array_size = 1024 * 1024;
	UINT32 data_size = 4;
	AM_MEM_FUNCTION_T amFa;
	AM_FUNC_CALLS_T *aCalls;
	UINT32 *localArray ;
	UINT32 i;
	UINT32 temp;
	INIT_OS_HR_TIMER(0);
	double elap1, elap2;
	UINT32 handle;
	UINT32 random_ops = 10000000;
	UINT32 rval, val, idx;
	UINT32 running_val = 0;

	OS_HR_TIME_PRINT_TIMER_RES();

	memcpy(&aCap, pCap, sizeof(aCap));

	if(aCap.typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE] & 0x4)
	{
		aCap.typeSpecific[TS_STAT_ARRAY_IDX_BYTE_SIZE] = 0x4;
		aCap.maxSize = array_size;
		aCap.typeSpecific[TS_STAT_ARRAY_VAL_DATA_TYPES] = TS_STAT_DT_FIXED_WIDTH;
		aCap.typeSpecific[TS_STAT_ARRAY_VAL_MAX_SIZE] = 4;

	}
	else
	{
		printf("NO 32bit Index Size\n");
		return;
	}

	aCalls = (AM_FUNC_CALLS_T *)AM_MALLOC( sizeof(am_fn) * aCap.functionCount);
	amFa.fn = aCalls;
	if(AM_RET_GOOD != pEntry->create_function(pEntry, &aCap, &amFa))
	{
		printf("Create Function Error\n");
		AM_FREE(aCalls);
		return;
	}
	
	handle = amFa.handle;
	localArray = (UINT32 *) AM_MALLOC(array_size * data_size);

	if(localArray)
	{
		OS_HR_TIMER_START();

		for(i = 0; i < array_size; i++)
		{
			temp = array_size - i;
			localArray[i] = temp;
		}
		OS_HR_TIMER_STOP();
		elap1 = OS_HR_TIMER_GET_ELAP();
		printf("LOCAL ARRAY WRITE %d entries ELAP = %f\n", array_size, elap1);

	}
	else
	{
		printf("Local Array Error\n");
		return;
	}


	OS_HR_PRINT_START_TIME();
	OS_HR_PRINT_STOP_TIME();
	OS_HR_TIMER_START();

	for(i = 0; i < array_size; i++)
	{
		temp = array_size - i;
		aCalls->write_al(handle, &i, &temp);
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("APMEM ARRAY WRITE %d entries ELAP = %f\n", array_size, elap2);
	printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	

	srand(100);
	idx = 0;
	val = 0;
	
	OS_HR_TIMER_START();
	for( i = 0; i < random_ops; i++)
	{
		rval = rand();
		idx = (val + rval) % array_size;
		val = localArray[idx]; 
		running_val += val;
	}
	OS_HR_TIMER_STOP();
	elap1 = OS_HR_TIMER_GET_ELAP();
	printf("LOCAL READ %d ops ELAP = %f\n", random_ops, elap1);

	srand(100);
	idx = 0;
	val = 0;

	OS_HR_TIMER_START();
	for( i = 0; i < random_ops; i++)
	{
		rval = rand();
		idx = (val + rval) % array_size;
		aCalls->read_al(handle,&idx, &val);
		running_val += val;
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("APMEM READ %d ops ELAP = %f\n", random_ops, elap2);
	printf("DELTA = %f PERCENT (Running Val=%d)\n",  100 * ((elap1 - elap2) / elap1), running_val);	

	free(localArray);
	free(aCalls);
}



char ** am_get_test_keys(char **pKeys, UINT32 key_count, UINT32 key_size)
{
	UINT32 i, j;
	char *pK;
	UINT8 offset;

	pKeys = AM_MALLOC(sizeof(char *) * key_count);

	for(i = 0; i < key_count; i++)
	{
		pK = AM_MALLOC(key_size);

		for(j = 0; j < (key_size - 1); j++)
		{
			offset = rand() % 26;
			pK[j] = 'A' + offset;
		}
		pK[key_size - 1] = '\0';

//		printf("KEY[%d] = %s\n", i, pK);
		pKeys[i] = pK;
	}
	return pKeys;


}

void am_test_assc_array(AM_MEM_CAP_T *pCap, AMLIB_ENTRY_T *pEntry)
{
	AM_MEM_CAP_T aCap;
	char **pKeys = NULL;
	UINT32 key_count = 1000;
	UINT32 key_size = 32;
	char *pK;
	UINT32 i;
	AMLIB_ASSCA *pAA;
	INIT_OS_HR_TIMER(0);
	double elap1, elap2;
	UINT32 random_ops = 10000000;
	UINT32 idx;
	UINT32 val;
	AM_MEM_FUNCTION_T amSa;
	AM_FUNC_CALLS_T *aCalls;
	UINT32 handle = 0;

	pKeys = am_get_test_keys(pKeys, key_count, key_size);

	if(pKeys)
	{

		memcpy(&aCap, pCap, sizeof(aCap));

		if((aCap.typeSpecific[TS_ASSCA_KEY_MAX_SIZE] >= key_size) &&
			(aCap.typeSpecific[TS_ASSCA_DATA_MAX_SIZE] >=  sizeof(UINT32)))

		{
			aCap.typeSpecific[TS_ASSCA_KEY_MAX_SIZE] = key_size;
			aCap.typeSpecific[TS_ASSCA_DATA_MAX_SIZE] =  sizeof(UINT32);
			aCap.typeSpecific[TS_ASSCA_KEY_TYPE] =  TS_ASSCA_KEY_FIXED_WIDTH;
			aCap.typeSpecific[TS_ASSCA_DATA_TYPE] =  TS_ASSCA_DATA_FIXED_WIDTH;
		
		}
		else
		{
			printf("No Key/Data Sizes %d:%d\n", key_size, sizeof(UINT32));
			return;
		}
		
		aCalls = (AM_FUNC_CALLS_T *)AM_MALLOC( sizeof(am_fn) * aCap.functionCount);
		amSa.fn = aCalls;
		
		if(AM_RET_GOOD != pEntry->create_function(pEntry, &aCap, &amSa))
		{
			printf("Create Function Error\n");
			AM_FREE(aCalls);
			handle = amSa.handle;
			return;
		}
	

		/* Since C doesn't have an associtive array type */
		/* We'll use our library functions directly */
		/* The only diference between VIRTD and base would be the API overhead */
		/* It will be a good measure of teh driver overhead of APPMEMD */
		
		OS_HR_TIMER_START();
		pAA = amlib_assca_init(key_size, 4, TRUE, TRUE, 0);
		
		for(i = 0; i < key_count; i++)
		{
			pK = pKeys[i];
			amlib_assca_add_key_fixfix(pAA, pK, &i);

		}
		OS_HR_TIMER_STOP();
		elap1 = OS_HR_TIMER_GET_ELAP();
		printf("Assc Array Write %d entries ELAP = %f\n", key_count, elap1);
	


		OS_HR_TIMER_START();
		for(i = 0; i < key_count; i++)
		{
			pK = pKeys[i];
		
			amSa.fn->write_al(handle, pK, &i);
			
		}
		OS_HR_TIMER_STOP();
		elap2 = OS_HR_TIMER_GET_ELAP();
		printf("%s Assc Array Write %d entries ELAP = %f\n", pEntry->am_name, key_count, elap2);













		OS_HR_TIMER_START();
		for(i = 0; i < random_ops; i++)
		{
			idx = rand() % key_count;		
			pK = pKeys[idx];

			if(AM_RET_GOOD == amlib_assca_get_key_val(pAA, pK, &val))
			{
				if(val != idx)
				{
					printf("ERROR Key VAL MISMATCH  %d=%d\n", val, idx);
					break;
			
				}
			}
			else
			{
				printf("ERROR Key not found =%s\n", pK);
				break;
			}

		}
		OS_HR_TIMER_STOP();
		elap1 = OS_HR_TIMER_GET_ELAP();
		printf("Assc Array Read %d ops ELAP = %f\n", random_ops, elap1);

	}
}

void am_test(AM_MEM_CAP_T *pAmCaps, UINT32 cap_count, UINT32 test, AMLIB_ENTRY_T *pEntry)
{
	UINT32 i;
	AM_MEM_CAP_T *pTestCap = NULL;

	for(i = 0; i < cap_count; i++)
	{
		pTestCap = &pAmCaps[i];
		if(pTestCap->amType == test)
		{
			break;
		}
		else
		{
			pTestCap = NULL;
		}

	}
	
	if(pTestCap)
	{
		switch(test)
		{
			case AM_TYPE_FLAT_MEM:
				am_test_flat_mem(pTestCap, pEntry);
			break;

			case AM_TYPE_ARRAY:
				am_test_static_array(pTestCap, pEntry);
				break;
			case AM_TYPE_ASSOC_ARRAY:
				am_test_assc_array(pTestCap, pEntry);
				break;

	
			default:
				printf("NO TEST CASE\n");
			break;

		}

	}
	else
	{
		printf("NO TEST CAPABILITY FOUND FOR TEST = 0x%08x\n");
	}



}


int main(int argc, char **argv)
{
//	UINT32 test = AM_TYPE_ARRAY;
	
	UINT32 test = AM_TYPE_ASSOC_ARRAY;
	char *driver_name = NULL;
	UINT32 cap_count = 0;
	AM_MEM_CAP_T *pAmCaps;
	UINT32 i;
	char pbuff[256];
	AMLIB_ENTRY_T	amEntry;

	printf("SIZEOF UINT64 = %d\n", sizeof(UINT64));
	printf("SIZEOF UINT32 = %d\n", sizeof(UINT32));
	printf("SIZEOF UINT16 = %d\n", sizeof(UINT16));
	printf("SIZEOF UINT8 = %d\n", sizeof(UINT8));
	printf("ARGC = %d\n", argc);

	if(argc > 1)
	{
		driver_name = argv[1];	
	}

	if(argc > 2)
	{
		test = strtol(argv[2], NULL, 10);
	}
	printf("App Mem Test Type %d\n", test);

	
	if(driver_name)
	{
		printf("APP_MEM DRIVER = %s\n", driver_name);
	}
	else
	{
		printf("APP_MEM DRIVER = VIRTD\n");
	}
	
	if(am_get_entry_point(driver_name, &amEntry))
	{
		printf("Invalid Entry Point %s\n", driver_name);
		return 0;
	}


	/* TEMP */
//	driver_name = NULL;

	cap_count = amEntry.get_cap_count(&amEntry);
	
	printf("CAP COUNT = %d \n", cap_count);

	if(cap_count)
	{
		pAmCaps = (AM_MEM_CAP_T *)AM_MALLOC(sizeof(AM_MEM_CAP_T) * cap_count);

		printf("SIZEOF CAP_BUFF ALLOC(%d) = %d\n", cap_count, sizeof(AM_MEM_CAP_T) * cap_count);

		if(NULL != pAmCaps)
		{
			if(AM_RET_GOOD == amEntry.get_capabilities(&amEntry, pAmCaps, cap_count))
			{
				//cap_count--; //TEMP

				for(i = 0; i < cap_count; i++)
				{

					printf("---- CAP #%d\n", i);
					am_sprintf_capability( &pAmCaps[i], (char *)pbuff, sizeof(pbuff));
					printf("CAP #%d\n%s\n", i, pbuff);
					
				}
			
				am_test(pAmCaps, cap_count, test, &amEntry);



			
			}
			else
			{
				printf("GET CAPS ERROR\n");
			}

		}
	

	}






	return 0;
}