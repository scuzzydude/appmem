
#include <cstdlib>
#include <stdio.h>
#include "appmemcpp.h"

#include "am_test_os.h"

extern "C" 
{
#include "am_assca.h"
char ** am_get_test_keys(char **pKeys, UINT32 key_count, UINT32 key_size);

}
#define USE_OPERATORS 1


int am_cpp_flat_mem_test(char *am_name)
{
	UINT32 random_ops = 10000000;
	UINT32 i;
	UINT32 mem_size = 1024 * 1024;
	INIT_OS_HR_TIMER(0);
	UINT32 *ptr32;
	UINT32 *pBase;
	double elap1, elap2;

	CAppMemFlat amFlat(am_name, 1024 * 1024, 4);
	printf("Appmem C++ - Flat Mem Test %s\n", am_name);

	if(amFlat.ready())
	{
		pBase = new UINT32[mem_size / sizeof(UINT32)];
		ptr32 = pBase;
	
		OS_HR_TIMER_START();
		for( i = 0; i < mem_size ; i += sizeof(UINT32))
		{
			*ptr32 = i;
			ptr32++;
		}
		OS_HR_TIMER_STOP();
		elap1 = OS_HR_TIMER_GET_ELAP();
		printf("(APPMEMCPP)IN-MEM RAW WRITE %d bytes ELAP = %f\n", mem_size, elap1);


		
		
		OS_HR_TIMER_START();
		for( i = 0; i < mem_size ; i += 4)
		{
			//amFmf.fn->write_al(handle, &i, &i);
			amFlat.write32(i, i);

		}
		OS_HR_TIMER_STOP();
		elap2 = OS_HR_TIMER_GET_ELAP();
		printf("(APPMEMCPP)%s RAW WRITE %d bytes ELAP = %f\n", amFlat.driverName() , mem_size, elap2);
		printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	

	
	
		UINT32 val = 0;
		UINT32 rval = 0;
		UINT32 offset;
		srand(100);
		OS_HR_TIMER_START();
		for( i = 0; i < random_ops; i++)
		{
			offset = (((val + rval) % mem_size) & ~3);   
			ptr32 = (UINT32 *)((UINT64)pBase + offset); 
			rval = rand();
			val = *ptr32;
			
			if(val != offset)
			{
				printf("Miscompare address %p val=0x%08x base=%p offset=0x%08x\n", ptr32, val, pBase, offset);
				break;
			}
		
		
		}
		OS_HR_TIMER_STOP();
		elap1 = OS_HR_TIMER_GET_ELAP();
		printf("(APPMEMCPP) RAW READ %d ops ELAP = %f\n", random_ops, elap1);


		val = 0;
		rval = 0;
		srand(100);
		UINT32 elem_cnt = amFlat.count();

		OS_HR_TIMER_START();
		for( i = 0; i < random_ops; i++)
		{
#if USE_OPERATORS
			offset = (((val + rval) % elem_cnt));   
			rval = rand();
			val = amFlat[offset];
#else
			offset = (((val + rval) % mem_size) & ~3);   
			rval = rand();
			amFlat.read32(offset, &val);
#endif			

#if USE_OPERATORS
			if(val != (offset * sizeof(UINT32)))
#else
			if(val != offset)
#endif

			{
				printf("Miscompare address %p val=0x%08x base=%p offset=0x%08x\n", ptr32, val, pBase, offset);
				break;
			}
		
		
		}
		OS_HR_TIMER_STOP();
		elap2 = OS_HR_TIMER_GET_ELAP();
		printf("(APPMEMCPP) %s READ %d ops ELAP = %f\n", amFlat.driverName(), random_ops, elap2);
		printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	






	}




	return 0;

}

int am_cpp_stata_test(char *am_name)
{
	UINT32 random_ops = 10000000;
	UINT32 i;
	UINT32 elem_count = 1024 * 1024;
	INIT_OS_HR_TIMER(0);
//	UINT32 *ptr32;
//	UINT32 *pBase;
	double elap1, elap2;
	UINT32 *localArray;
	UINT32 temp;
	

	CAppMemStaticArray amStata(am_name, elem_count, sizeof(UINT32), sizeof(UINT32));

	printf("Appmem C++ - Flat Mem Test %s\n", am_name);


	localArray = new UINT32[elem_count];

	if(localArray)
	{
		OS_HR_TIMER_START();

		for(i = 0; i < elem_count; i++)
		{
			temp = elem_count - i;
			localArray[i] = temp;
		}
		OS_HR_TIMER_STOP();
		elap1 = OS_HR_TIMER_GET_ELAP();
		printf("(APPMEMCPP)LOCAL ARRAY WRITE %d entries ELAP = %f\n", elem_count, elap1);

	}
	else
	{
		printf("Local Array Error\n");
		return 0;
	}


	OS_HR_TIMER_START();

	for(i = 0; i < elem_count; i++)
	{
		temp = elem_count - i;
		amStata.insert(i, &temp);	
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("(APPMEMCPP)%s ARRAY WRITE %d entries ELAP = %f\n", amStata.driverName(), elem_count, elap2);
	printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	


	srand(100);
	UINT32 idx = 0;
	UINT32 val = 0;
	UINT32 rval;
	UINT32 running_val = 0;

	srand(100);
	OS_HR_TIMER_START();
	for( i = 0; i < random_ops; i++)
	{
		rval = rand();
		idx = (val + rval) % elem_count;
		val = localArray[idx]; 
		temp = elem_count - idx;
		
		if(temp != val)
		{
			printf("Miscompare idx %d val=0x%08x temp=0x%08x\n", idx, val, temp);
			break;
		}


		running_val += val;
	}
	OS_HR_TIMER_STOP();
	elap1 = OS_HR_TIMER_GET_ELAP();
	printf("(APPMEMCPP)LOCAL READ %d ops ELAP = %f\n", random_ops, elap1);

	idx = 0;
	val = 0;
	rval;

	srand(100);
	OS_HR_TIMER_START();
	for( i = 0; i < random_ops; i++)
	{
		rval = rand();
		idx = (val + rval) % elem_count;
		amStata.get(idx, &val);
		temp = elem_count - idx;
		
		if(temp != val)
		{
			printf("Miscompare idx %d val=0x%08x temp=0x%08x\n", idx, val, temp);
			break;
		}


		running_val += val;
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("(APPMEMCPP)%s READ %d ops ELAP = %f\n", amStata.driverName(), random_ops, elap2);
	printf("DELTA = %f PERCENT\n",  100 * ((elap1 - elap2) / elap1));	



	printf("running_val=%d\n", running_val);
	delete localArray;
	return 0;
}


int am_cpp_assca_test(char *am_name)
{
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
	
	CAppMemAsscArray amAssca(am_name, key_size, sizeof(UINT32), true, true);

	pKeys = am_get_test_keys(pKeys, key_count, key_size);

	if(NULL == pKeys)
	{
		return 0;
	}

	/* TODO: Use C++ Container templates for comparison */
	/* Just do the same as C example for now */

	pAA = amlib_assca_init(key_size, 4, TRUE, TRUE, 0);
	OS_HR_TIMER_START();
	for(i = 0; i < key_count; i++)
	{
		pK = pKeys[i];
		amlib_assca_add_key_fixfix(pAA, pK, &i);

	}
	OS_HR_TIMER_STOP();
	elap1 = OS_HR_TIMER_GET_ELAP();
	printf("(APPMEMCPP) Local Assc Array Write %d entries ELAP = %f\n", key_count, elap1);



	OS_HR_TIMER_START();
	for(i = 0; i < key_count; i++)
	{
		pK = pKeys[i];
		amAssca.insert(pK, &i);		
	}
	OS_HR_TIMER_STOP();
	elap2 = OS_HR_TIMER_GET_ELAP();
	printf("(APPMEMCPP) %s Assc Array Write %d entries ELAP = %f\n", amAssca.driverName(), key_count, elap2);





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
	printf("(APPMEMCPP) Local Assc Array Read %d ops ELAP = %f\n", random_ops, elap1);






	return 0;

}

extern "C" int am_cpp_test(char *am_name, UINT32 test)
{
	if(AM_TYPE_FLAT_MEM == test)
	{
		return	am_cpp_flat_mem_test(am_name);
	}
	else if(AM_TYPE_ARRAY == test)
	{
		return am_cpp_stata_test(am_name);
	}
	else if(AM_TYPE_ASSOC_ARRAY == test)
	{
		return am_cpp_assca_test(am_name);
	}
	return 0;

}



