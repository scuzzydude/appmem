#include <stdio.h>
#include <stdlib.h>
#include "appmemlib.h"
#include "am_test_os.h"

void am_test_flat_mem(AM_MEM_CAP_T *pCap, char *driver_name)
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

	func_buf = malloc( sizeof(am_fn) * aCap.functionCount);
	amFmf.fn = func_buf;
	if(AM_RET_GOOD != am_create_function(driver_name, &aCap, &amFmf))
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


	local_buf = (UINT32 *)malloc(mem_size);

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

void am_test_static_array(AM_MEM_CAP_T *pCap, char *driver_name)
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

	aCalls = (AM_FUNC_CALLS_T *)malloc( sizeof(am_fn) * aCap.functionCount);
	amFa.fn = aCalls;
	if(AM_RET_GOOD != am_create_function(driver_name, &aCap, &amFa))
	{
		printf("Create Function Error\n");
		return;
	}
	
	handle = amFa.handle;
	localArray = malloc(array_size * data_size);

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

void am_test_assc_array(AM_MEM_CAP_T *pCap, char *driver_name)
{

}

void am_test(AM_MEM_CAP_T *pAmCaps, UINT32 cap_count, UINT32 test, char *driver_name)
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
				am_test_flat_mem(pTestCap, driver_name);
			break;

			case AM_TYPE_ARRAY:
				am_test_static_array(pTestCap, driver_name);
				break;
			case AM_TYPE_ASSOC_ARRAY:
				am_test_assc_array(pTestCap, driver_name);
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
	
	UINT32 test AM_TYPE_ASSOC_ARRAY
	char *driver_name = NULL;
	UINT32 cap_count = 0;
	AM_MEM_CAP_T *pAmCaps;
	UINT32 i;
	char pbuff[256];

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
	
	/* TEMP */
	driver_name = NULL;

	cap_count = am_get_capabilities_count(driver_name);

	printf("CAP COUNT = %d cap_count\n");

	if(cap_count)
	{
		pAmCaps = (AM_MEM_CAP_T *)malloc(sizeof(AM_MEM_CAP_T) * cap_count);

		if(NULL != pAmCaps)
		{
			if(AM_RET_GOOD == am_get_capabilities(driver_name, pAmCaps, cap_count))
			{

				for(i = 0; i < cap_count; i++)
				{

					am_sprintf_capability( &pAmCaps[i], (char *)pbuff, sizeof(pbuff));
					printf("CAP #%d\n%s\n", i, pbuff);
					
				}
			
				am_test(pAmCaps, cap_count, test, driver_name);



			
			}
			else
			{
				printf("GET CAPS ERROR\n");
			}

		}
	

	}






	return 0;
}