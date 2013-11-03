
#include <cstdlib>
#include <stdio.h>
#include "appmemcpp.h"

#include "am_test_os.h"

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
	printf("Appmem C++ Test %s\n", am_name);

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
			ptr32 = (UINT32 *)((UINT32)pBase + offset); 
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

extern "C" int am_cpp_test(char *am_name, UINT32 test)
{
	if(AM_TYPE_FLAT_MEM == test)
	{
		return	am_cpp_flat_mem_test(am_name);
	}
	return 0;

}
