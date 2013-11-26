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
#include <stdio.h>
#include <stdlib.h>
#include "appmemlib.h"
#include "am_test_os.h"
#include "am_assca.h"

#define APPMEMCPP_ONLY	

UINT32 gTestMemSize = 1024;
UINT32 gTestRandomOps = 1000;


//#define DEFAULT_MEM_SIZE 16 
//#define DEFAULT_RANDOM_OPS 10

#ifndef _WIN32
double get_linux_ts_elap(struct timespec *pTs1, struct timespec *pTs2)
{
	double elap;
	double elap1 = (double)pTs1->tv_sec + ( (double)pTs1->tv_nsec / (double)(1000 * 1000 * 1000));
	double elap2 = (double)pTs2->tv_sec + ( (double)pTs2->tv_nsec / (double)(1000 * 1000 * 1000));


//	printf("TS1 = %d.%d = %f\n", pTs1->tv_sec, pTs1->tv_nsec, elap1);
//	printf("TS2 = %d.%d = %f\n", pTs2->tv_sec, pTs2->tv_nsec, elap2);


	elap = elap2 - elap1;
	
	return elap;
}
#endif



int am_cpp_test(char *am_name, UINT32 test);

void am_test_flat_mem(AM_MEM_CAP_T *pCap, AMLIB_ENTRY_T *pEntry)
{

	AM_MEM_CAP_T aCap;
	void *func_buf;
	AM_MEM_FUNCTION_T amFmf;
	UINT32 mem_size = gTestMemSize;
	UINT32 i;
	UINT32 val;
	UINT32 *ptr32;
	UINT32 *local_buf;
	double elap1, elap2;
	INIT_OS_HR_TIMER(0);
	UINT32 random_ops = gTestRandomOps;
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
		
	
	if(AM_RET_GOOD != amFmf.fn->open(&amFmf))
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
		amFmf.fn->write_al(&amFmf, &i, &i);
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("WRITE %d bytes ELAP = %f\n", mem_size, elap2);
	printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	

	val = 0;
	rval = 0;
	printf("Start read test ....\n");

	OS_HR_TIMER_START();
	for( i = 0; i < random_ops; i++)
	{
		addr =  (((val + rval) % mem_size) & ~3);
		
	//	printf("i=%x addr=%x mem_size=%x val=%x rval=%x\n", i, addr, mem_size, val, rval); 
		
		ptr32 = (UINT32 *)((UINT64)local_buf + (UINT64)addr); 
	
	//	printf("i=%x ptr32=%p local_buf=%p\n", i, ptr32, local_buf); 
	
		rval = rand();
		val = *ptr32;
		
		if(val != addr)
		{
			printf("Miscompare address %p val=0x%08x base=%p offset=0x%08x\n", ptr32, val, local_buf, addr);
			break;

		}

	
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
		rval = rand();
		amFmf.fn->read_al(&amFmf, &addr, &val);
		
		if(val != addr)
		{
			printf("(appmem) Miscompare address %p val=0x%08x base=%p offset=0x%08x\n", ptr32, val, local_buf, addr);
			break;

		}


	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("READ %d ops  = %f\n", random_ops, elap2);
	printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	


	



	if(AM_RET_GOOD != amFmf.fn->close(&amFmf))
	{
		printf("Close Function Error\n");
		return;
	}


}

void am_test_static_array(AM_MEM_CAP_T *pCap, AMLIB_ENTRY_T *pEntry)
{
	AM_MEM_CAP_T aCap;
	UINT32 array_size = gTestMemSize;
	UINT32 data_size = 4;
	AM_MEM_FUNCTION_T amFa;
	AM_FUNC_CALLS_T *aCalls;
	UINT32 *localArray ;
	UINT32 i;
	UINT32 temp;
	INIT_OS_HR_TIMER(0);
	double elap1, elap2;
	UINT32 random_ops = gTestRandomOps;
	UINT32 rval, val, idx;
	UINT32 running_val = 0;
	AM_SORT_PARAM_U sortP;

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

	aCalls = (AM_FUNC_CALLS_T *)AM_MALLOC(sizeof(AM_FUNC_CALLS_T));

    memset(aCalls, 0, (sizeof(AM_FUNC_CALLS_T)));
    
	amFa.fn = aCalls;
	if(AM_RET_GOOD != pEntry->create_function(pEntry, &aCap, &amFa))
	{
		printf("Create Function Error\n");
		AM_FREE(aCalls);
		return;
	}
	

	if(AM_RET_GOOD != amFa.fn->open(&amFa))
	{
		printf("Open Function Error\n");
		return;
	}


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
		aCalls->write_al(&amFa, &i, &temp);
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
		temp = array_size - idx;
		running_val += val;
		if(temp != val)
		{
			printf("MISCOMPARE IDX=%d VAL=%d TEMP=%d\n", idx, val, temp);
			break;
		}

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
		aCalls->read_al(&amFa,&idx, &val);
		temp = array_size - idx;
		running_val += val;
		if(temp != val)
		{
			printf("MISCOMPARE IDX=%d VAL=%d TEMP=%d\n", idx, val, temp);
			break;
		}


	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("APMEM READ %d ops ELAP = %f\n", random_ops, elap2);
	printf("DELTA = %f PERCENT (Running Val=%d)\n",  100 * ((elap1 - elap2) / elap1), running_val);	
	sortP.stata_integral.type = AM_SORT_TYPE_STATA_INTEGRAL_MERGE;
	sortP.stata_integral.order = AM_SORT_ORDER_ASC;
	sortP.stata_integral.start_idx = 0;
	sortP.stata_integral.end_idx = 0; //sort all if 0
	sortP.stata_integral.data_signed = 0;

    if(NULL != aCalls->sort)
    {
    
	    printf("STATA Array Sort %s sortfn=%p\n", amFa.crResp.am_name, aCalls->sort);

	    OS_HR_TIMER_START();
	    if(AM_RET_GOOD != aCalls->sort(&amFa, &sortP, sizeof(AM_SORT_PARAM_U)))
	    {
		    printf("Sort Error\n");
	    }

	    OS_HR_TIMER_STOP();
	    elap2 = OS_HR_TIMER_GET_ELAP();
	    printf("STATA Array Sort %s Elap=%f\n", amFa.crResp.am_name, elap2);
    }
	
	
	
	
	free(localArray);
	free(aCalls);
}



char ** am_get_test_keys(char **pKeys, UINT32 key_count, UINT32 key_size)
{
	UINT32 i, j;
	char *pK;
	UINT8 offset;

	pKeys = (char **)AM_MALLOC(sizeof(char *) * key_count);

	for(i = 0; i < key_count; i++)
	{
		pK = (char *) AM_MALLOC(key_size);

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
	UINT32 key_count = gTestMemSize;
	UINT32 key_size = 32;
	char *pK;
	UINT32 i;
	AMLIB_ASSCA *pAA;
	INIT_OS_HR_TIMER(0);
	double elap1, elap2;
	UINT32 random_ops = gTestRandomOps;
	UINT32 idx;
	UINT32 val;
	UINT32 iter_handle;
	char ikey[32];
	UINT32 ival;
	int count;


	AM_MEM_FUNCTION_T amAA;
	AM_FUNC_CALLS_T *pCalls;

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
		
	    pCalls = (AM_FUNC_CALLS_T *)AM_MALLOC(sizeof(AM_FUNC_CALLS_T));

        memset(pCalls, 0, (sizeof(AM_FUNC_CALLS_T)));
        
		amAA.fn = pCalls;
		
		if(AM_RET_GOOD != pEntry->create_function(pEntry, &aCap, &amAA))
		{
			printf("Create Function Error\n");
			AM_FREE(pCalls);
			return;
		}
		

	    if(AM_RET_GOOD != amAA.fn->open(&amAA))
	    {
		    printf("Open Function Error\n");
		    return;
	    }

		/* Since C doesn't have an associtive array type */
		/* We'll use our library functions directly */
		/* The only diference between VIRTD and base would be the API overhead */
		/* It will be a good measure of teh driver overhead of APPMEMD */
		
		pAA = amlib_assca_init(key_size, 4, TRUE, TRUE, 0);
		OS_HR_TIMER_START();
		
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
//		    printf("pK[%d] = %s func=%p\n", i, pK, amAA.fn->write_al);
			amAA.fn->write_al(&amAA, pK, &i);
			
		}

		OS_HR_TIMER_STOP();
		elap2 = OS_HR_TIMER_GET_ELAP();
		printf("%s Assc Array Write %d entries ELAP = %f\n", pEntry->am_name, key_count, elap2);
	    printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	

		OS_HR_TIMER_START();
		for(i = 0; i < random_ops; i++)
		{
			idx = rand() % key_count;		
			pK = pKeys[idx];

			if(AM_RET_GOOD == amlib_assca_get_key_val(pAA, pK, &val))
			{
				if(val != idx)
				{
					printf("ERROR Key[%s] VAL MISMATCH  %d=%d\n", pK, val, idx);
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



		OS_HR_TIMER_START();
		for(i = 0; i < random_ops; i++)
		{
			idx = rand() % key_count;		
			pK = pKeys[idx];
            val = 0;
    
//			printf("READ pK[%d] = %s \n", idx, pK);
	
			if(AM_RET_GOOD == amAA.fn->read_al(&amAA, pK, &val))
			{
				if(val != idx)
				{
					printf("ERROR Key[%s] VAL MISMATCH  %d=%d VALPTR=%p\n", pK, val, idx, &val);
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
		elap2 = OS_HR_TIMER_GET_ELAP();
		printf("%s Assc Array Read %d ops ELAP = %f\n", pEntry->am_name, random_ops, elap2);
        printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	


        if(NULL != amAA.fn->iter)
        {

		    iter_handle = ASSCA_ITER_FLAG_NEW;
	    	printf("BEGIN ITER %s HANDLE=%08x\n", pEntry->am_name, iter_handle);
		    count = 0;
		    while(AM_RET_GOOD == amAA.fn->iter(&amAA, &ikey[0], key_size, &ival, sizeof(UINT32), &iter_handle))
		    {
//			    printf("ITER(%d) KEY=%s VAL=%d\n", count, ikey, ival);
			    count++;

		    }
        }


		if(NULL != amAA.fn->release)
		{
			amAA.fn->release(&amAA, NULL);
			
		}



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
//	UINT32 test = AM_TYPE_FLAT_MEM;

	char *driver_name = NULL;
//	char *driver_name = "127.0.0.1";
//	char *driver_name = "192.168.1.221";

	UINT32 cap_count = 0;
	AM_MEM_CAP_T *pAmCaps;
	UINT32 i;
	char pbuff[256];
	AMLIB_ENTRY_T	amEntry;



	if(argc <= 1)
	{
		printf("appmemtest <appmemdevice> <amType> <test elements> <random ops> <C/C++/Both>\n");
		printf("\n");
		
		printf("<appmemdevice>\n"); 
		printf("virtd             - Userspace Library Emulation\n");
		printf("/dev/appmem       - Kernel Driver Emulation\n");
		printf("xxx.xxx.xxx.xxx   - IP of an am_targ\n");
		printf("\n");
		
		printf("<amType>\n");
		printf("1                 - Flat Memory\n");
		printf("2                 - Static Array\n");
		printf("3                 - Associative Array [DEFAULT]\n");
		printf("\n");

		
		return 0;
	}

	if(argc > 1)
	{
		driver_name = argv[1];	

        if(0 == strcmp(argv[1], "virtd"))
        {
            driver_name = NULL;
        }
	}
	if(argc > 2)
	{
		test = strtol(argv[2], NULL, 10);
	}
	printf("App Mem Test Type %d\n", test);


	AM_DEBUGPRINT("SIZEOF UINT64 = %d\n", sizeof(UINT64));
	AM_DEBUGPRINT("SIZEOF UINT32 = %d\n", sizeof(UINT32));
	AM_DEBUGPRINT("SIZEOF UINT16 = %d\n", sizeof(UINT16));
	AM_DEBUGPRINT("SIZEOF UINT8 = %d\n", sizeof(UINT8));
	AM_DEBUGPRINT("SIZEOF size_t = %d\n", sizeof(size_t));
	AM_DEBUGPRINT("SIZEOF void * = %d\n", sizeof(void *));
	AM_DEBUGPRINT("ARGC = %d\n", argc);

	if(driver_name)
	{
		printf("APP_MEM DRIVER = %s\n", driver_name);
	}
	else
	{
		printf("APP_MEM DRIVER = VIRTD\n");
	}
#ifndef APPMEMCPP_ONLY	
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

	amEntry.close(&amEntry);
#endif
	am_cpp_test(driver_name, test);

	return 0;
}
